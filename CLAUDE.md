# LastStand — 프로젝트 가이드 (Claude)

멀티플레이어 3인칭 슈터. Unreal Engine 5 C++ 모듈 `LastStand`.

## 엔진 / 빌드

- **소스 빌드 엔진**: `D:/WorkSpace/UnrealSource/UnrealEngine` (EngineAssociation GUID `{B50F3950-4338-ED68-80F8-3683FDFF3943}`, `HKCU\Software\Epic Games\Unreal Engine\Builds`에 등록).
- **새 소스 파일(.h/.cpp)을 추가하면 항상 솔루션(프로젝트 파일)을 재생성**한다. 기존 파일만 수정하면 불필요.
  ```bash
  & "D:\WorkSpace\UnrealSource\UnrealEngine\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="C:\Workspace\LastStand\LastStand.uproject" -game -engine -progress
  ```
- 모듈 의존성(`LastStand.Build.cs`)에 이미 포함: Core/CoreUObject/Engine/InputCore/EnhancedInput/UMG/GameplayTags/NetCore/Slate/SlateCore, (Private) OnlineSubsystem.

## 네이밍 / 언어 컨벤션

- 클래스 접두사 뒤 **`LS`**: `ALSPlayerCharacter`, `ULSStatComponent`, `FWeaponData`, `ELS… / EWeaponMontageType`, `ILSStatComponentInterface`.
- 코드 주석은 **한국어**. 대화도 한국어. **커밋 메시지 본문은 영어**.
- `UMETA(DisplayName = ...)`는 **영어**로.
- UE 타입 사용: `int32`/`FString`/`TArray`/`TMap`, `TObjectPtr<>`(헤더 UPROPERTY 포인터), `#pragma once`.

## 네트워크 패턴 (이 프로젝트의 핵심)

서버 권위 + 로컬 예측. 반복적으로 쓰는 패턴:

- **로컬 예측 + Server RPC + 리플리케이션**: 소유 클라가 즉시 로컬 반영(반응성) → `UFUNCTION(Server, Reliable)`로 서버 반영 → 서버가 프로퍼티 리플리케이션으로 타 클라에 전파.
- **소유자 제외 복제**: 소유 클라가 로컬로 이미 세팅한 상태는 `DOREPLIFETIME_CONDITION(Class, Var, COND_SkipOwner)`로 owner에게 되돌리지 않음 (예: `bIsAim`).
- **상태 플래그**: `UPROPERTY(ReplicatedUsing = OnRep_X)` + `OnRep_X`에서 클라 측 후처리/델리게이트 브로드캐스트. OnRep은 **클라에서만** 발화(서버 authority 제외) → "서버 제외 모든 클라 실행"이 필요하면 OnRep이 정확한 훅 (예: 장착 몽타주 `OnRepIsActived`).
- **RPC 신뢰성**: 상태 전환(엣지 트리거)은 `Reliable` (드롭 시 고착 방지). 고빈도 연출(발사/재장전 몽타주 멀티캐스트)은 `Unreliable` 허용.
- **멀티캐스트 중복 방지**: 발사 몽타주 등은 로컬(소유 클라)에서 즉시 재생 후 `NetMulticast`로 전파하되, `OwnerCh->IsLocallyControlled()`인 소유 클라는 스킵.
- **연사 속도 가드**: 서버는 `GetWorld()->GetTimeSeconds()` 타임스탬프 + `IntervalTime * 0.9f` 허용오차(`LastFireServerTime`).
- **파괴 시 정리**: 언이큅은 `ALSEquipmentBase::UnEquipped()`가 `Destroy()` → 모든 머신 `EndPlay` → 여기서 안전하게 정리(예: 애니 레이어 언링크).

## 컴포넌트 / 데이터 / 인터페이스 패턴

- **컴포넌트 인터페이스 컨벤션**: `UINTERFACE(MinimalAPI) U…Interface` + `class LASTSTAND_API I…Interface`에 **순수 가상**(`= 0`) 게터/함수. 인터페이스 .cpp에는 로직을 두지 않고 **액터가 구현**(예: `ILSStatComponentInterface::GetStatComponent()`, `ApplyDamage()`는 `ALSCharacterBase`/`ALSEnemyBase`가 구현해 컴포넌트로 위임). 데미지 등 실제 로직은 **받는 쪽 컴포넌트**(`ULSStatComponent`)에 둔다.
- **인터페이스 소비**: `Actor->Implements<U…Interface>()` → `Cast<I…Interface>(Actor)->Fn()` (예: [LSDropItem.cpp](Source/LastStand/Props/LSDropItem.cpp), 데미지 적용).
- **데이터 접근**: `GetOwner()->GetGameInstance()->GetSubsystem<ULSDataSubsystem>()` → `FindItem/FindWeapon/FindEnemy` (`UDataTable::FindRow<FRow>`). 테이블은 `ULSGameDataSettings`(Config=Game)에서 로드.
- **컴포넌트**: 생성자에서 `CreateDefaultSubobject`, 리플리케이트 컴포넌트는 `SetIsReplicatedByDefault(true)`. 델리게이트는 **비다이나믹** `DECLARE_MULTICAST_DELEGATE[_NParams]` + `AddUObject`/`RemoveAll` (예: `FOnHealthChanged`, `FOnEquipmentArrayUpdated`).
- **에셋 참조**: 하드 레퍼런스 금지. `TSoftObjectPtr`/`TSoftClassPtr` + 사용 시점 `LoadSynchronous()`.
- **UI 위젯 컴포넌트**: 위젯 컴포넌트 위젯은 붙은 pawn을 self-discovery 어려움 → **pawn이 `GetUserWidgetObject()`로 위젯을 얻어 `InitializeWidget(Comp)` 주입**. 데디 서버는 위젯 미생성이라 자동 스킵. 위젯 갱신은 `OnHealthChanged` 등 델리게이트 구독(클라).
- **UI 이벤트**: `ULSUIEventSubsystem`의 델리게이트(`InventoryInput`, `DamageEvent(int32)`)로 게임플레이 → HUD 연동. `GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>()`.

## 소스 구조

`Source/LastStand/` 하위: `Character/`(+`Components/`), `AI/`, `Item/Equipment/Weapon/`, `UI/`(+`Widget/`,`Operation/`), `Data/`, `DataTable/`, `Interface/`, `Settings/`, `Player/`, `GameState/`, `Props/`, `Animation/`, `Save/`.

## Git / 커밋 워크플로우

- 브랜치 `main`에 직접 커밋·푸시(사용자가 요청 시). git user: `wonjin`.
- **기능 단위로 커밋 분리**(관련 없는 변경은 별도 커밋). 커밋 본문은 영어, 마지막에 `Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>`.
- 흐름: `git status`/`git diff --stat`로 확인 → 기능별 `git add <files>` → 커밋 → `git push origin main`. 신규 파일 커밋 후엔 솔루션 재생성.
- Windows 환경이라 `LF will be replaced by CRLF` 경고는 정상(무시).

## 작업 스타일 (사용자 피드백)

- **플랜 모드**를 적극 사용. 계획을 세밀히 검토하며, 변수명/로직 위치/설계를 자주 다듬음 → 기존 컨벤션에 맞추는 것을 선호.
- 기능 구현 후 **빌드는 하지 않고** 사용자가 에디터에서 확인하는 경우가 많음. 구현 완료 후 "빌드/커밋할까요?"로 확인.
- 사용자가 스캐폴드/변수를 직접 추가해두고 그 위에 구현을 요청하는 경우가 많음 → 되돌릴 때는 **사용자 추가분은 보존**하고 내 변경만 원복.

## 시스템 인덱스 (코드베이스 맵)

향후 작업의 진입점 빠른 조회용. 경로는 `Source/LastStand/` 기준(플러그인은 별도 명시). 파일명은 접두사 `LS` 생략 없이 표기.

### WFC 절차적 맵 생성 — `Plugins/MapGenerator/Source/MapGenerator/`
- `MapGeneratorSubsystem.*` (~900줄, 핵심): `UWorldSubsystem`. 파이프라인 오케스트레이션.
  - `GenerateHeightGrid`(펄린 fBm) → `FindLocalMaxima`(극댓값 시드) → `BuildVoronoiRegions`(**JFA 보로노이**, 핑퐁 버퍼) → `MarkRegionBoundaries`(BFS 경계) → `QuantizeHeights`(계단화).
  - `GenerateTileGrid` = Stage B 오케스트레이터: `BuildTileSet`(소켓 FName→비트인덱스 인터닝) → `ComputeColumnLevels`(3D 볼륨화) → `RunWFC`.
  - `RunWFC`(line ~630): 소켓 비트마스크 교집합 인접, `Compatible[dir][tile]` 사전계산, 커스텀 `TArray<uint32>` 비트셋(익명 `WFCBits` ns), Observe(최소엔트로피+리저버샘플)→Collapse(최대 Weight)→Propagate(워크리스트), 모순 시 `MaxAttempts=20` 리시드 재시도, `FRandomStream`으로 결정성.
- `MapGrid.h`(FMapGrid 높이/영역), `MapTileGrid.h`(FWFCTile 런타임 구조체 + FMapTileGrid 결과), `MapAssetData.h`(FMapAssetData 타일 행: 6면 소켓/Weight/Mesh), `MapData.h`(FMapData 노이즈·WFC 파라미터 행), `DataTableSettings.h`(UDeveloperSettings).
- `MapGridVisualizer.*`(AMapGridVisualizer): 타일 타입별 **HISM** 지연 생성, per-axis `FVector CellSize` 인스턴싱.
- 게임 모듈 의존성: `LastStand.Build.cs`에 `MapGenerator` 등록.

### 네트워크 전투 (랙 보상)
- `Character/Components/LSServerSideRewindComponent.*`: 서버 전용 틱, 20ms 스냅샷 링버퍼(200ms 윈도우), `ConfirmHit(start,end,ts)` = 브래킷 스냅샷 보간(Lerp+Slerp)→세그먼트-OBB 판정. 히트박스는 `ILSHitboxInterface`로 제네릭 수집.
- `Character/Components/LSHitboxComponent.*`: `UBoxComponent` 파생, `ELSHitboxType`+`DamageMultiplier`(Head 2.0/WeakPoint 3.0/사지 0.7), 소켓 재부착. `ProcessLocalHit`(예측) vs `ProcessServerHit`(권위).
- `Item/Equipment/Weapon/LSWeaponHitscan.*`: 로컬 예측 트레이스 → `ServerRPCFire(Start,End,HitActor,Timestamp)` → 서버 연사가드(`Interval*0.9`)+리와인드 재검증+권위 데미지 → 멀티캐스트 연출(소유 클라 스킵). 커스텀 채널 `ECC_GameTraceChannel1`(Hitscan).

### 캐릭터 / 컴포넌트
- `Character/LSCharacterBase.*`(ACharacter, Stat/Inventory/Interact 인터페이스 구현, 컨트롤 회전 리플리케이트), `LSPlayerCharacter.*`(Enhanced Input, `bIsAim` `COND_SkipOwner` 예측, 다수 ServerRPC).
- `Character/Components/`: `LSStatComponent`(권위 HP, `OnRep_CurrentHealth`+이중 브로드캐스트), `LSEquipmentComponent`(리플리 장비맵+탄약캐시, `OnRepFocusEquipment` 애님레이어 링크), `LSInventoryComponent`(FastArray, `OnInventoryItemChanged` 델타), `LSInteractionComponent`.
- `AI/LSEnemyBase.*`(APawn, 7개 소켓 히트박스+리와인드+월드 HP바 위젯, 서버 애님 강제 틱), `AI/LSAIController.*`(BehaviorTree 구동).

### 데이터 / 인터페이스 / UI
- `Settings/LSGameDataSettings`(UDeveloperSettings, soft DataTable refs) → `DataTable/LSDataSubsystem`(`FindItem/FindWeapon/FindEnemy`) → 행 구조체 `LSItemData`/`LSWeaponData`/`LSEnemyData`(모두 FTableRowBase).
- `Interface/`: `LSStatComponentInterface`(GetStatComponent+ApplyDamage), `LSHitboxInterface`(GetHitboxComponents), `LSInventoryComponentInterface`, `LSInteractComponentInterface`, `LSInteractableInterface`(Interact/GetItemName/GetInstanceID).
- `UI/LSUISubsystem`(GameplayTag 레이어/슬롯, `PendingInjections` 지연 주입, 입력모드) + `UI/LSUIEventSubsystem`(이벤트 버스: Health/Ammo/Damage/Equipment/ReserveAmmo/InventoryInput). `UI/Widget/`(인벤/장비/스탯/상호작용), `UI/Operation/`(드래그드롭), 데미지넘버 오브젝트 풀링(`LSDamageLayerWidget`).
- `Item/LSItemArray.h`: `FInventoryItemInfoArray`(FFastArraySerializer, WithNetDeltaSerializer).

### 스캐폴드/미완 (건드릴 때 주의)
- `Save/LSSaveSubsystem`·`LSSaveGame`: 빈 스텁. AI 행동트리: BP/데이터 애셋 기반(커스텀 C++ BT 노드 없음).

### 포트폴리오 README
- 루트 `README.md`은 **개발자 포트폴리오용**(한국어). WFC·리와인드·네트워킹·아키텍처 중심 서술 + WFC mermaid 다이어그램. 시스템 변경 시 README 해당 섹션도 함께 갱신 고려.
