# LastStand

언리얼 엔진 5 · C++로 만든 서버 권위(Server-Authoritative) 멀티플레이 3인칭 슈터입니다. 개인 프로젝트로 설계와 C++ 게임플레이/네트워크/툴 구현을 전반적으로 진행했습니다.

주로 다룬 부분은 절차적 던전 맵 생성, 네트워크 기반 TPS 공격 로직(서버사이드 리와인드), 그리고 현재 작업 중인 멀티플레이 보스 AI입니다. 던전 생성과 랙 보상은 UE 기본 제공 기능이 아니라 직접 구현했습니다.

사용 기술: UE5(소스 빌드), C++, Enhanced Input, GameplayTags, Online Subsystem, UMG.

<!-- 대표 GIF/스크린샷 삽입 위치 (생성된 던전 & 인게임 플레이) -->

---

## 절차적 던전 맵 생성

여러 방(서브레벨)을 문으로 이어 붙여 던전을 만드는 절차적 생성기를 독립 Runtime 플러그인 `MapGenerator`로 분리했습니다. 게임 모듈과 디커플되어 재사용·독립 컴파일이 가능하고, 생성 로직은 `UWorldSubsystem`에 둡니다. 에디터 전용 툴링은 별도 Editor 모듈(`MapGeneratorEditor`)로 분리했습니다.

- 데이터 드리븐 방 정의: 각 방은 `FMapData` 행(방 서브레벨 `TSoftObjectPtr<UWorld>`, `ERoomType` 시작/도착/일반/통로, 문 배열 — `Doors[0]`=입구·`Doors[1..]`=출구, 경계 박스 목록 `BoundsBoxes`)으로 기술합니다.
- 백트래킹 DFS 배치(`BuildChain`): 시작 방을 원점에 두고, 각 출구 문에서 모든 방 후보를 랜덤 순서로 시도합니다. 배치 가능하면 그 방으로 내려가고(재귀), 막히면 다음 후보·다음 문으로 되돌아갑니다(백트래킹). 이어진 방 수가 `RoomCount`에 도달하면 마지막 방에서 도착 방을 배치하고 종료합니다. `FRandomStream(Seed)`로 같은 시드는 항상 같은 결과를 냅니다.
- 문 맞물림 배치(`ComputeChildTransform`): 자식 입구 문이 부모 출구 문의 반대 방향을 향하도록 90° 단위로 회전하고, 두 문의 월드 위치가 일치하도록 평행이동해 방을 이어 붙입니다.
- 박스볼륨 충돌 회피(`CanPlaceRoom`): 방 경계를 표시하는 `ABoxVolume`을 각 방 서브레벨에 배치하고, 그 바운즈를 데이터 테이블 행(`BoundsBoxes`)으로 미리 export합니다. 배치 시 후보 방의 world AABB가 기존 방들과 겹치면(접촉은 허용하는 Epsilon 여유) 거부해 백트래킹을 유도합니다.
- 에디터 툴링: `AMapGenerator` 액터가 생성 파라미터 데이터 에셋(`UMapGenerationData`: 참조 테이블·방 개수·시드)을 멤버로 들고, `CallInEditor` 버튼(Generate·Clear Rooms)으로 생성·프리뷰합니다. 바운즈 export는 콘텐츠 브라우저의 DataTable 우클릭 메뉴로 실행합니다 — `UDeveloperSettings`의 CallInEditor 버튼은 편집 대상이 CDO(archetype)라 실행되지 않는 엔진 제약이 있어, 에디터 모듈에서 에셋 컨텍스트 메뉴로 우회했습니다.
- 서브레벨 스트리밍: 결정된 배치를 `ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr`로 방 레벨별 인스턴스로 스트리밍해 배치합니다.

```cpp
// 자식 입구(Doors[0])가 부모 출구의 반대 방향을 향하고, 두 문의 월드 위치가 일치하도록 배치
const float ChildYaw = YawOfDir(OppositeDir(ParentExitWorldDir)) - YawOfDir(ChildEntrance.Direction);
const FRotator Rot(0.f, ChildYaw, 0.f);
const FVector  T = ParentExitWorldPos - Rot.RotateVector(ChildEntrance.Location);
return FTransform(Rot.Quaternion(), T);
```

관련 코드: [MapGeneratorSubsystem.h](Plugins/MapGenerator/Source/MapGenerator/Public/MapGeneratorSubsystem.h) · [.cpp](Plugins/MapGenerator/Source/MapGenerator/Private/MapGeneratorSubsystem.cpp), [MapData.h](Plugins/MapGenerator/Source/MapGenerator/Public/MapData.h), [MapGenerationData.h](Plugins/MapGenerator/Source/MapGenerator/Public/MapGenerationData.h), [MapGeneratorActor.h](Plugins/MapGenerator/Source/MapGenerator/Public/MapGeneratorActor.h) · [.cpp](Plugins/MapGenerator/Source/MapGenerator/Private/MapGeneratorActor.cpp), [BoxVolume.h](Plugins/MapGenerator/Source/MapGenerator/Public/BoxVolume.h), [MapGeneratorEditor.cpp](Plugins/MapGenerator/Source/MapGeneratorEditor/Private/MapGeneratorEditor.cpp)

<!-- 생성 파이프라인 GIF 삽입 위치 (방 연결 → 충돌 회피 → 스트리밍) -->

---

## 네트워크 기반 TPS 공격 로직 구현

발사는 클라에서 먼저 처리하고, 판정과 데미지는 서버가 다시 확인합니다.

1. 소유 클라가 바로 트레이스하고 몽타주·FX·데미지 넘버를 띄웁니다.
2. `ServerRPCFire(Start, End, HitActor)`로 서버에 알립니다. 발사 시각은 보내지 않습니다.
3. 서버는 연사 간격(`Interval * 0.9`)을 확인하고 탄약을 차감한 뒤, 리와인드로 명중을 재검증해 데미지를 적용합니다.
4. 연출은 `NetMulticast Unreliable`로 다른 클라에 보내고, 이미 재생한 소유 클라는 건너뜁니다.

### 서버사이드 리와인드

- 서버가 20ms마다 히트박스 위치·회전·크기를 기록하고 200ms까지 보관합니다.
- 되감을 시간은 서버가 측정한 핑(RTT)을 씁니다. 클라가 조작할 수 없고, 최대 200ms로 제한합니다.
- 해당 시점 앞뒤 스냅샷을 보간(`Lerp`/`Slerp`)한 뒤 레이와 박스(OBB)의 교차를 검사합니다.
- 히트박스는 `ILSHitboxInterface`로 가져오기 때문에 플레이어와 적에 같은 코드를 씁니다.
- 데디 서버에서도 본 위치가 맞도록 `AlwaysTickPoseAndRefreshBones`를 사용합니다.

```cpp
// 리와인드로 확인된 히트박스에만 데미지 적용 (부위 배율 포함)
if (ULSServerSideRewindComponent* SSR = HitActor->GetComponentByClass<ULSServerSideRewindComponent>())
{
    if (ULSHitboxComponent* Hitbox = SSR->ConfirmHit(TraceStart, TraceEnd, Cast<APawn>(GetOwner())))
    {
        Hitbox->ProcessServerHit(Damage);
    }
}
```

관련 코드: [LSWeaponHitscan.h](Source/LastStand/Item/Equipment/Weapon/LSWeaponHitscan.h) · [.cpp](Source/LastStand/Item/Equipment/Weapon/LSWeaponHitscan.cpp), [LSServerSideRewindComponent.h](Source/LastStand/Character/Components/LSServerSideRewindComponent.h) · [.cpp](Source/LastStand/Character/Components/LSServerSideRewindComponent.cpp), [LSHitboxComponent.h](Source/LastStand/Character/Components/LSHitboxComponent.h) · [.cpp](Source/LastStand/Character/Components/LSHitboxComponent.cpp)

---

## 멀티플레이 보스 AI (작업 중)

> 🚧 현재 개발 중인 시스템입니다. 타겟 선정(어그로)과 StateTree 연동까지 구현했고, 행동·공격 패턴은 계속 확장하고 있습니다.

여러 플레이어가 동시에 상대하는 보스를 위해, 타겟 판단은 서버에서만 하고 선정 규칙은 데이터로 바꿀 수 있게 설계했습니다.

- 서버 전용 감지([`ULSAIPerceptionComponent`](Source/LastStand/AI/Components/LSAIPerceptionComponent.h)): `UCapsuleComponent`를 상속해 QueryOnly·Pawn 오버랩만 받는 감지 볼륨입니다. 오버랩 바인딩과 틱은 `HasAuthority()`일 때만 켜고 클라에서는 틱을 끕니다. 플레이어(`ALSCharacterBase`)만 후보로 모으고, 범위를 벗어나면 즉시 재선정합니다.
- 데이터 드리븐 어그로: `FEnemyData` 행의 `ELSTargetSelectType`(`Closest` / `TopDamage` / `Random` / `RandomLocked`)과 `TargetChangeDelay`로 적마다 선정 방식을 정합니다. 딜레이 동안은 현재 타겟을 유지해 타겟이 떨리지 않게 하고, 타겟이 파괴되거나 범위를 벗어나면 딜레이를 무시하고 즉시 재선정합니다. `RandomLocked`는 한 번 고른 타겟을 잃기 전까지 고정합니다.
- 누적 데미지 기반 어그로: `ULSStatComponent::ApplyDamage`가 데미지 주체별 누적량을 `TMap<TWeakObjectPtr<AActor>, int32>`에 기록하고, `GetTopDamageCauser()`로 가장 많이 때린 플레이어를 찾습니다. 아직 피격이 없거나 그 플레이어가 범위 밖이면 최근접으로 대체합니다.
- StateTree 연동: 커스텀 Global Task(`FLSStateTreeCombatGlobalTask`)가 퍼셉션 컴포넌트를 캐싱하고 선정된 `TargetActor`를 Output으로 노출합니다. 하위 전투 상태는 이 값에 바인딩만 하면 되므로 타겟 선정 로직과 행동 로직이 분리됩니다. EQS 기반 위치 선정은 작업 중입니다.

```cpp
// 타겟 상실(파괴/범위 이탈)이면 딜레이와 무관하게 즉시 재선정
const bool bTargetLost = !IsValid(TargetActor) || !PerceivedActors.Contains(TargetActor);

// 랜덤 후 고정: 타겟을 잃기 전까지 유지
if (TargetSelectType == ELSTargetSelectType::RandomLocked && !bTargetLost)
{
    return;
}

// 변경 딜레이 안에서는 현재 타겟 유지
if (!bTargetLost && Now - LastTargetChangeTime < TargetChangeDelay)
{
    return;
}
```

관련 코드: [LSAIPerceptionComponent.h](Source/LastStand/AI/Components/LSAIPerceptionComponent.h) · [.cpp](Source/LastStand/AI/Components/LSAIPerceptionComponent.cpp), [LSStateTreeCombatGlobalTask.h](Source/LastStand/AI/StateTree/LSStateTreeCombatGlobalTask.h) · [.cpp](Source/LastStand/AI/StateTree/LSStateTreeCombatGlobalTask.cpp), [LSEnemyData.h](Source/LastStand/DataTable/LSEnemyData.h), [LSStatComponent.h](Source/LastStand/Character/Components/LSStatComponent.h) · [.cpp](Source/LastStand/Character/Components/LSStatComponent.cpp)

---

## 엔진 아키텍처 / 데이터 드리븐

- 컴포넌트 조합 + 인터페이스 분리: `ALSCharacterBase`는 Stat/Inventory/Interact 인터페이스를, `ALSEnemyBase`는 Stat/Hitbox 인터페이스를 구현합니다. 그래서 무기나 리와인드 같은 공용 시스템이 공통 베이스 클래스 없이 하나의 코드 경로로 플레이어와 적을 모두 다룹니다.
- 데이터 드리븐 코어: `UDeveloperSettings`(`ULSGameDataSettings`) → `ULSDataSubsystem`(`FindItem`/`FindWeapon`/`FindEnemy`) → `FTableRowBase` 행 조회 구조입니다. 에셋은 하드 레퍼런스를 쓰지 않고 `TSoftObjectPtr`/`TSoftClassPtr`로 참조한 뒤 사용 시점에 `LoadSynchronous` 합니다.
- 인벤토리/장비: `FFastArraySerializer` 델타 인벤토리, 리플리케이트 장비 맵(TMap는 배열로 우회 복제 후 클라에서 재구성), 인벤토리 델타를 구독하는 리플리케이트 탄약 캐시로 구성됩니다.
- 태그 기반 레이어드 UI: `ULSUISubsystem`(`FGameplayTag` 키 레이어/슬롯, 미등록 슬롯용 지연 주입 큐, 입력 모드 관리)과 `ULSUIEventSubsystem`(멀티캐스트 이벤트 버스)으로 게임플레이와 HUD를 분리했습니다. 데미지 넘버는 오브젝트 풀링을 씁니다.
- 무기 연출: 무기별 애님 레이어 링크(`LinkAnimClassLayers`), 런타임 커브 생성과 `FTimeline` 기반 ADS 줌, `EWeaponMontageType` 맵 기반 몽타주 디스패치(역재생 지원)를 구현했습니다.

관련 코드: [LSCharacterBase.h](Source/LastStand/Character/LSCharacterBase.h) · [.cpp](Source/LastStand/Character/LSCharacterBase.cpp), [LSEnemyBase.h](Source/LastStand/AI/LSEnemyBase.h) · [.cpp](Source/LastStand/AI/LSEnemyBase.cpp), [LSGameDataSettings.h](Source/LastStand/Settings/LSGameDataSettings.h) · [.cpp](Source/LastStand/Settings/LSGameDataSettings.cpp), [LSDataSubsystem.h](Source/LastStand/DataTable/LSDataSubsystem.h) · [.cpp](Source/LastStand/DataTable/LSDataSubsystem.cpp), [LSUISubsystem.h](Source/LastStand/UI/LSUISubsystem.h) · [.cpp](Source/LastStand/UI/LSUISubsystem.cpp), [LSUIEventSubsystem.h](Source/LastStand/UI/LSUIEventSubsystem.h) · [.cpp](Source/LastStand/UI/LSUIEventSubsystem.cpp), [LSEquipmentComponent.h](Source/LastStand/Character/Components/LSEquipmentComponent.h) · [.cpp](Source/LastStand/Character/Components/LSEquipmentComponent.cpp), [LSInventoryComponent.h](Source/LastStand/Character/Components/LSInventoryComponent.h) · [.cpp](Source/LastStand/Character/Components/LSInventoryComponent.cpp)

### 리플리케이션 패턴

| 패턴 | 용도 | 예시 |
|------|------|------|
| `ReplicatedUsing = OnRep_X` | 상태 전파와 클라 후처리. 서버는 OnRep이 발화하지 않으므로 직접 브로드캐스트 | `CurrentHealth`, `CurrentAmmo` |
| `COND_SkipOwner` | 소유 클라가 이미 예측한 상태는 되돌리지 않고 타 클라에만 전파 | `bIsAim` |
| `Reliable` RPC | 상태 전환(엣지 트리거). 드롭 시 고착 방지 | `ServerRPCAim`, `ServerRPCReload` |
| `Unreliable` Multicast | 고빈도 연출(발사/재장전 몽타주·FX) | `MulticastRPCPlayFireEffects` |
| `FFastArraySerializer` | 인벤토리 배열 델타 리플리케이션 | `FInventoryItemInfoArray` |

관련 코드: [LSPlayerCharacter.h](Source/LastStand/Character/LSPlayerCharacter.h) · [.cpp](Source/LastStand/Character/LSPlayerCharacter.cpp)

---

## 프로젝트 구조

```
LastStand/
├─ Source/LastStand/
│  ├─ Character/          플레이어 캐릭터·베이스
│  │  └─ Components/      Stat / Equipment / Inventory / Interaction / Hitbox / ServerSideRewind
│  ├─ AI/                 EnemyBase(Character) + AIController, Components/(Perception), StateTree/(Global Task)
│  ├─ Item/Equipment/Weapon/   장비·무기 베이스, 히트스캔 무기
│  ├─ UI/                 UISubsystem·이벤트버스·레이어/슬롯 위젯, Widget/, Operation/
│  ├─ Data/               CharacterControl·Weapon·GameModeInfo 데이터 애셋
│  ├─ DataTable/          DataSubsystem + Item/Weapon/Enemy 행 구조체
│  ├─ Interface/          Stat/Hitbox/Inventory/Interact/Interactable 인터페이스
│  ├─ Settings/           GameDataSettings (UDeveloperSettings)
│  ├─ Player/ GameState/ Gamemode/ Props/ Animation/ Save/ Tags/
└─ Plugins/MapGenerator/  방 기반 절차적 던전 생성 (독립 Runtime + Editor 모듈)
```

바로가기: [Character](Source/LastStand/Character) · [Components](Source/LastStand/Character/Components) · [AI](Source/LastStand/AI) · [Weapon](Source/LastStand/Item/Equipment/Weapon) · [UI](Source/LastStand/UI) · [DataTable](Source/LastStand/DataTable) · [Interface](Source/LastStand/Interface) · [Settings](Source/LastStand/Settings) · [MapGenerator](Plugins/MapGenerator/Source/MapGenerator)

---

## 개발 환경

- 엔진: 언리얼 엔진 5 소스 빌드
- 모듈 의존성: Core / CoreUObject / Engine / InputCore / EnhancedInput / UMG / GameplayTags / NetCore / Slate / SlateCore, (Private) OnlineSubsystem
- 게임플레이 관련 코드 주석은 한국어로 작성
