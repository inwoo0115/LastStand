# LastStand

> **언리얼 엔진 5 · C++ 서버 권위(Server-Authoritative) 멀티플레이 3인칭 슈터**
> 개인 프로젝트 — 설계 및 C++ 게임플레이/네트워크/툴 전반 구현

`UE5 (소스 빌드)` · `C++` · `Enhanced Input` · `GameplayTags` · `Online Subsystem` · `UMG`

멀티플레이 슈터에 필요한 핵심 엔지니어링을 UE 기본 제공 기능에 의존하지 않고 직접 설계·구현한 프로젝트입니다. 절차적 맵 생성(WFC), 랙 보상(서버사이드 리와인드), 서버 권위 + 클라이언트 예측 네트워킹을 중심으로 합니다.

<!-- 대표 GIF/스크린샷 삽입 위치 (WFC로 생성된 맵 & 인게임 플레이) -->

---

## ✨ 핵심 하이라이트

- **🗺️ 절차적 맵 생성 (WFC)** — 펄린 fBm 지형 → **JFA(Jump Flooding) 보로노이** 영역 후처리 → **소켓 비트마스크 Wave Function Collapse 솔버**를 독립 플러그인으로 직접 구현. 최소 엔트로피 휴리스틱 + 모순 복구 재시도 + 시드 결정성.
- **🎯 랙 보상 (Server-Side Rewind)** — UE가 기본 제공하지 않는 히트박스 히스토리 기반 **랙 보상**을 직접 구현. 20ms 스냅샷 링버퍼 + 쿼터니언 보간 + 세그먼트-OBB 판정.
- **🌐 서버 권위 + 클라 예측** — RPC / 프로퍼티 리플리케이션 / `COND_SkipOwner` / OnRep 규율을 일관되게 적용한 반응성과 권위의 균형.

---

## 🗺️ 시스템 1 — WFC 절차적 맵 생성기 (Flagship)

전체가 **독립 Runtime 플러그인** `MapGenerator`로 분리되어 있어(게임 모듈과 디커플, 재사용·독립 컴파일 가능) `UWorldSubsystem`으로 레벨별 수명을 갖습니다.

### 2단계 파이프라인 (2D 높이 필드 → 3D 타일 솔브)

```mermaid
flowchart LR
    A["GenerateHeightGrid<br/>펄린 fBm"] --> B["FindLocalMaxima<br/>극댓값 시드"]
    B --> C["BuildVoronoiRegions<br/>JFA 보로노이"]
    C --> D["MarkRegionBoundaries<br/>BFS 경계"]
    D --> E["QuantizeHeights<br/>높이 계단화"]
    E --> F["ComputeColumnLevels<br/>3D 볼륨화"]
    F --> G["RunWFC<br/>제약 전파 솔버"]
    G --> H["HISM 인스턴싱<br/>메쉬 시각화"]
```

**Stage A — 높이 필드 생성 & 후처리**
- **fBm(fractional Brownian motion)**: 다중 옥타브 `PerlinNoise2D`, 옥타브별 시드 오프셋, `Persistence`(진폭 감쇠)/`Lacunarity`(주파수 증가), 진폭 합으로 정규화해 [-1, 1] 유지.
- **JFA 보로노이 영역화**: 8방향 극댓값을 시드로 잡고(인접 플래토 병합), **Jump Flooding Algorithm**(파워오브투 halving step + 핑퐁 버퍼)으로 각 셀을 최근접 시드 영역으로 라벨링. 제곱 유클리드 거리(int64, sqrt 없음).
- **경계 BFS**: 서로 다른 영역이 맞닿는 경계를 다중 소스 거리 제한 BFS로 두께만큼 확장해 "해자(moat)" 존 생성.
- **높이 양자화**: `GridSnap`으로 이산 계단화 → 타일 적층에 적합한 지형.

**Stage B — WFC 솔버 (`RunWFC`)**
- **소켓 비트마스크 인접**: 모든 소켓 `FName`을 비트 인덱스(0–63)로 인터닝, 타일 6면을 각각 `uint64` 마스크로 표현. **두 면은 마스크 교집합이 있으면 인접 허용**(`(a & b) != 0`) — 정확 일치보다 유연.
- **사전계산 호환 테이블** `Compatible[dir][tile]` + `TArray<uint32>` 워드 단위 **커스텀 비트셋**(`Bits &= Bits-1` 순회, `CountBits`/`CountTrailingZeros`)으로 제약 검사를 비트 연산으로 처리.
- **희소 솔브**: 열 높이로 솔리드 셀만 처리(전체 W×H×D 박스 아님). 경계 소켓(`Empty`/`Floor`) 조건을 초기 **아크 일관성 전파**로 안쪽에 전파.
- **Observe → Collapse → Propagate**: 최소 엔트로피(남은 후보 최소) 셀 선택(동률은 **리저버 샘플링**) → 최대 `Weight` 타일로 붕괴 → 워크리스트 기반 제약 전파 & 모순 감지.
- **모순 복구**: 붕괴 루프를 `MaxAttempts = 20` 재시도로 감싸고 매 시도 `FRandomStream(Seed + Attempt)` 리시드. `WFCSeed`로 **완전한 결정성** 보장.

```cpp
// WFC 솔버 내부 타일 표현 — FName은 최초 인터닝 시에만, hot loop는 전부 int/비트마스크
struct FWFCTile
{
    FName  RowName;                                 // 카탈로그 인덱스 → 원본 행(메쉬 조회)
    float  Weight = 1.0f;                           // 가중치(동률은 리저버 샘플링)
    uint64 SocketMasks[6] = { 0,0,0,0,0,0 };        // +X,-X,+Y,-Y,+Z,-Z 면 소켓 비트마스크
};
```

**시각화** — 타일 타입별 **HISM(Hierarchical Instanced Static Mesh)** 컴포넌트를 지연 생성하고, per-axis `FVector CellSize`로 X/Y 풋프린트와 Z 층 높이를 분리해 수천 개 타일을 효율적으로 인스턴싱.

📂 `Plugins/MapGenerator/Source/MapGenerator/` — `MapGeneratorSubsystem`, `MapGrid`, `MapTileGrid`, `MapAssetData`, `MapData`, `MapGridVisualizer`

<!-- WFC 파이프라인 단계별 시각화 GIF 삽입 위치 (노이즈 → 보로노이 → 타일 솔브) -->

---

## 🎯 시스템 2 — 서버사이드 리와인드 (랙 보상)

핑이 높은 클라이언트도 "쏜 순간 화면에 보이던 위치"로 명중 판정되도록, Valve/Overwatch식 **랙 보상**을 직접 구현했습니다.

- **히스토리 기록** (`ULSServerSideRewindComponent`, 서버 전용 틱): `RecordInterval = 20ms`마다 히트박스별 스냅샷(월드 위치/회전/스케일된 박스 크기)을 `TMap<FName, FHitBoxSnapshot>`으로 저장, `HistoryEndOffset = 200ms` 초과분은 FIFO 제거.
- **되감기 판정** (`ConfirmHit(start, end, timestamp)`): 발사 타임스탬프를 감싸는 두 스냅샷을 찾아 **위치/크기는 `Lerp`, 회전은 `Slerp`로 보간**(`InterpolateBox`) → 레이를 박스 로컬 공간으로 변환해 **세그먼트 vs OBB** 판정(`LineBoxIntersection`) → 매칭되는 **현재** 히트박스 반환.
- **제네릭 수집**: `ILSHitboxInterface::GetHitboxComponents()`로 히트박스를 추상 수집 → 플레이어/적 등 인터페이스만 구현하면 동작.
- **정확도 디테일**: 데디케이티드 서버는 렌더링이 없어도 소켓 부착 히트박스가 정확해야 하므로 `VisibilityBasedAnimTickOption = AlwaysTickPoseAndRefreshBones`로 본을 항상 갱신.

```cpp
// 서버: 클라가 보고한 피격 대상을 리와인드로 재검증한 뒤에만 권위 데미지 적용
if (ULSServerSideRewindComponent* SSR = HitActor->GetComponentByClass<ULSServerSideRewindComponent>())
{
    if (ULSHitboxComponent* Hitbox = SSR->ConfirmHit(TraceStart, TraceEnd, Timestamp))
    {
        Hitbox->ProcessServerHit(Damage);   // 부위 배율 적용 + 권위 데미지
    }
}
```

📂 `Source/LastStand/Character/Components/LSServerSideRewindComponent.*`, `LSHitboxComponent.*`

---

## 🌐 시스템 3 — 서버 권위 + 클라이언트 예측 네트워킹

반응성(로컬 예측)과 치트 방지(서버 권위)를 동시에 잡는 네트워크 규율을 일관되게 적용했습니다.

**히트스캔 발사 파이프라인**
1. **로컬 예측(소유 클라)**: 카메라 기준 스프레드 콘 트레이스 즉시 실행 → 발사 몽타주·머즐/임팩트 FX·예측 데미지 넘버 UI 즉시 반영.
2. **서버 RPC**: `ServerRPCFire(Start, End, HitActor, Timestamp)` — 서버 월드 시간 타임스탬프 동봉.
3. **서버 검증**: 연사 속도 가드(`Now - LastFire < Interval * 0.9`) → 탄약 차감 → **서버사이드 리와인드 재검증** → 권위 데미지.
4. **멀티캐스트 연출**: 몽타주/FX를 `NetMulticast Unreliable`로 전파하되 **소유 클라는 스킵**(이미 로컬 재생) → 이중 연출 방지.

**리플리케이션 패턴**

| 패턴 | 용도 | 예시 |
|------|------|------|
| `ReplicatedUsing = OnRep_X` | 상태 전파 + 클라 후처리. 서버는 OnRep 미발화이므로 직접 브로드캐스트 | `CurrentHealth`, `CurrentAmmo` |
| `COND_SkipOwner` | 소유 클라가 이미 예측한 상태는 되돌리지 않고 타 클라에만 전파 | `bIsAim` |
| `Reliable` RPC | 상태 전환(엣지 트리거), 드롭 시 고착 방지 | `ServerRPCAim`, `ServerRPCReload` |
| `Unreliable` Multicast | 고빈도 연출(발사/재장전 몽타주·FX) | `MulticastRPCPlayFireEffects` |
| `FFastArraySerializer` | 인벤토리 배열 델타 리플리케이션 | `FInventoryItemInfoArray` |

📂 `Source/LastStand/Item/Equipment/Weapon/LSWeaponHitscan.*`, `Character/LSPlayerCharacter.*`

---

## 🧩 시스템 4 — 엔진 아키텍처 & 데이터 드리븐

- **컴포넌트 조합 + 인터페이스 분리**: `ALSCharacterBase`가 Stat/Inventory/Interact 3개 인터페이스를, `ALSEnemyBase`가 Stat/Hitbox 인터페이스를 구현. 덕분에 무기·리와인드 등 공용 시스템이 **단일 코드 경로로 플레이어와 적을 모두** 처리(공통 베이스 클래스 불필요).
- **데이터 드리븐 코어**: `UDeveloperSettings(ULSGameDataSettings)` → `ULSDataSubsystem`(`FindItem`/`FindWeapon`/`FindEnemy`) → `FTableRowBase` 행 조회. 에셋은 **하드 레퍼런스 금지**, `TSoftObjectPtr`/`TSoftClassPtr` + 사용 시점 `LoadSynchronous`.
- **인벤토리/장비**: `FFastArraySerializer` 델타 인벤토리, 리플리케이트 장비 맵(TMap는 배열로 우회 복제 후 클라 재구성), 인벤토리 델타를 구독하는 리플리케이트 탄약 캐시.
- **태그 기반 레이어드 UI**: `ULSUISubsystem`(`FGameplayTag` 키 레이어/슬롯, 미등록 슬롯용 지연 주입 큐, 입력 모드 관리) + `ULSUIEventSubsystem`(멀티캐스트 **이벤트 버스**) → 게임플레이와 HUD 완전 디커플. 데미지 넘버는 **오브젝트 풀링**.
- **무기 연출**: 무기별 **애님 레이어 링크**(`LinkAnimClassLayers`), 런타임 커브 생성 + `FTimeline` 기반 **ADS 줌**, `EWeaponMontageType` 맵 기반 몽타주 디스패치(역재생 지원).

---

## 📁 프로젝트 구조

```
LastStand/
├─ Source/LastStand/
│  ├─ Character/          플레이어 캐릭터·베이스
│  │  └─ Components/      Stat / Equipment / Inventory / Interaction / Hitbox / ServerSideRewind
│  ├─ AI/                 EnemyBase(Pawn) + AIController(BehaviorTree 구동)
│  ├─ Item/Equipment/Weapon/   장비·무기 베이스, 히트스캔 무기
│  ├─ UI/                 UISubsystem·이벤트버스·레이어/슬롯 위젯, Widget/, Operation/
│  ├─ Data/               CharacterControl·Weapon·GameModeInfo 데이터 애셋
│  ├─ DataTable/          DataSubsystem + Item/Weapon/Enemy 행 구조체
│  ├─ Interface/          Stat/Hitbox/Inventory/Interact/Interactable 인터페이스
│  ├─ Settings/           GameDataSettings (UDeveloperSettings)
│  ├─ Player/ GameState/ Gamemode/ Props/ Animation/ Save/ Tags/
└─ Plugins/MapGenerator/  WFC 절차적 맵 생성 (독립 Runtime 플러그인)
```

---

## 🛠️ 개발 환경

- **엔진**: 언리얼 엔진 5 **소스 빌드**
- **모듈 의존성**: Core / CoreUObject / Engine / InputCore / EnhancedInput / UMG / GameplayTags / NetCore / Slate / SlateCore, (Private) OnlineSubsystem
- 게임플레이 관련 코드 주석은 한국어로 작성

## 🚧 향후 작업

- **세이브 시스템**(`Save/LSSaveSubsystem`, `LSSaveGame`) — 스캐폴드 상태, 직렬화 로직 구현 예정
- **AI** — 현재 행동트리는 블루프린트/데이터 애셋 기반, 커스텀 C++ BT 노드 확장 여지
