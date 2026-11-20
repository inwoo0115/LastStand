# LastStand

언리얼 엔진 5(소스 빌드)와 C++로 만든 서버 권위 방식의 멀티플레이 3인칭 슈터입니다.

<!-- 대표 GIF -->

주요 구현

- [네트워크 전투](#1-네트워크-전투): 클라이언트 예측, 서버사이드 리와인드, CMC 이동 예측(달리기·대시·로프 액션)
- [보스 AI](#2-보스-ai): StateTree와 DataTable 기반의 서버 전용 AI
- [절차적 던전 생성](#3-절차적-던전-생성): 방 서브레벨을 문 단위로 연결하는 플러그인
- [UI 시스템](#4-ui-시스템): GameplayTag 레이어/슬롯, 이벤트 버스, 드래그 앤 드롭 인벤토리

기술 스택: UE5, C++, Dedicated Server, StateTree, Enhanced Input, GameplayTags, Niagara, UMG

---

## 1. 네트워크 전투

### 발사 흐름

```mermaid
flowchart LR
    A[클라 예측 발사] --> B[ServerRPCFire]
    B --> C[서버 리와인드 판정]
    C --> D[Multicast 연출]
```

- 소유 클라이언트가 먼저 트레이스, 몽타주, 이펙트, 데미지 넘버를 처리하고 `ServerRPCFire(Start, End, HitActor)`를 보냅니다.
- 서버는 연사 간격(`Interval * 0.9`)과 탄약을 확인한 뒤 리와인드로 명중을 다시 판정하고 데미지를 적용합니다.
- 연출은 `NetMulticast Unreliable`로 보내고, 이미 재생한 소유 클라이언트는 `IsLocallyControlled()`로 건너뜁니다.
- 히트스캔 전용 트레이스 채널을 두고 Pawn 프로파일에서는 무시하도록 해서, 캡슐이 아니라 부위별 히트박스(머리 2.0, 약점 3.0, 팔다리 0.7)에만 맞게 했습니다.

### 서버사이드 리와인드

- 서버에서 20ms마다 히트박스의 위치, 회전, 크기를 기록하고 200ms 동안 보관합니다.
- 되감는 시간은 클라이언트가 보낸 값이 아니라 서버가 측정한 쏜 사람의 핑(RTT)을 사용하며, 최대 200ms로 제한합니다.
- 해당 시점 앞뒤 스냅샷을 `Lerp`/`Slerp`로 보간하고, 레이를 박스 로컬 공간으로 옮겨 OBB 교차를 검사합니다.
- 히트박스는 `ILSHitboxInterface`로 가져오기 때문에 플레이어와 적에 같은 코드를 사용합니다.
- 부위 배율이 필요 없는 적은 `bUsePhysicsAsset`을 켜서 물리 에셋 바디로 판정할 수 있습니다. 판정할 때 바디를 과거 포즈로 옮겨 엔진 트레이스(`LineTraceComponent`)를 실행한 뒤 원래 포즈로 되돌립니다.
- 데디 서버에서도 본 위치가 갱신되도록 `AlwaysTickPoseAndRefreshBones`를 사용합니다.

```cpp
// 일부 생략
float RewindTime = Shooter->GetPlayerState()->GetPingInMilliseconds() * 0.001f;
const float QueryTime = Now - FMath::Clamp(RewindTime, 0.0f, HistoryEndOffset);

Alpha = (QueryTime - Before->Time) / (After->Time - Before->Time);
const FHitBoxSnapshot Box = InterpolateBox(Pair.Value, *BoxB, Alpha);
if (SegmentIntersectsBox(Box, TraceStart, TraceEnd)) { /* 히트박스 반환 */ }
```

### 이동 예측

- `UCharacterMovementComponent`를 상속해 달리기, 대시, 로프 액션을 별도 RPC 없이 CMC 예측 흐름 안에서 처리합니다.
- `FSavedMove_LS`에 입력을 저장하고 `FLAG_Custom_0~3` 압축 플래그(달리기/대시/로프 점프/스윙)로 서버에 보냅니다. 목표 위치나 앵커처럼 플래그에 담기지 않는 값은 커스텀 `FCharacterNetworkMoveData`로 move와 함께 보냅니다.
- 대시는 `FRootMotionSource_ConstantForce`를 사용해 서버 보정과 리플레이에 포함됩니다.
- 요청이 있는 move는 `CanCombineWith`에서 합치지 않습니다.
- 리플레이 중에 현재 프레임 입력이 과거 값으로 덮이지 않도록 `ClientUpdatePositionAfterServerUpdate` 전후로 입력을 보존합니다.

### 로프 액션

- 소유 클라이언트가 매 틱 시선 방향으로 구체 스윕을 해서 타깃을 판정합니다. 바닥에 맞으면 그 지점, 벽에 맞으면 모서리 위를 아래로 탐색해 착지 후보를 찾고, 캡슐이 설 공간이 있으면 **착지 점프**, 없고 앵커가 충분히 높으면 **스윙**으로 분기합니다.
- 착지 점프는 커스텀 루트모션 소스 `FLSRootMotionSource_ParabolicMoveTo`(직선 보간 + 포물선)로 이동합니다. 매 틱 "이번 틱이 끝날 때 있어야 할 위치"로 속도를 계산하므로 오차가 스스로 보정됩니다.
- 서버는 클라이언트가 보낸 착지점을 그대로 믿거나 거부하지 않고, 자기 시점에서 다시 찾은 착지점으로 점프를 확정합니다. 조준의 미세한 차이 때문에 거부되어 출발점으로 되돌아가던 문제를 없앴습니다.
- 클라이언트 궤적은 서버 보정 한 번으로 서버 궤적에 맞춥니다. `Matches`(같은 점프인지 식별)는 목표를 비교하지 않고, `MatchesAndHasSameState`(상태 비교)는 시작·목표·높이를 비교하도록 나눴습니다. 목표가 다르면 상태 불일치로 재시뮬레이션이 일어나고, `UpdateStateFrom`이 서버 궤적을 복사합니다.
- 스윙은 커스텀 무브먼트 모드(`PhysSwing`)입니다. 중력, 방향키 가속, 로프 길이를 넘은 만큼의 장력, 로프 방향 성분만의 감쇠를 더하고, `SafeMoveUpdatedComponent`로 이동하며 벽에 막히면 미끄러집니다.

```cpp
// 일부 생략
bool FLSRootMotionSource_ParabolicMoveTo::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
    // 궤적이 다르면 다른 상태 → 클라가 재시뮬레이션하며 서버 궤적을 받음
    const auto* OtherCast = static_cast<const FLSRootMotionSource_ParabolicMoveTo*>(Other);
    return FRootMotionSource::MatchesAndHasSameState(Other)
        && FVector::PointsAreNear(TargetLocation, OtherCast->TargetLocation, 1.0f)
        && FMath::IsNearlyEqual(ArcHeight, OtherCast->ArcHeight, 1.0f);
}
```

관련 코드: [LSWeaponHitscan.h](Source/LastStand/Item/Equipment/Weapon/LSWeaponHitscan.h) · [.cpp](Source/LastStand/Item/Equipment/Weapon/LSWeaponHitscan.cpp), [LSServerSideRewindComponent.h](Source/LastStand/Character/Components/LSServerSideRewindComponent.h) · [.cpp](Source/LastStand/Character/Components/LSServerSideRewindComponent.cpp), [LSHitboxComponent.h](Source/LastStand/Character/Components/LSHitboxComponent.h) · [.cpp](Source/LastStand/Character/Components/LSHitboxComponent.cpp), [LSCharacterMovementComponent.h](Source/LastStand/Character/Components/LSCharacterMovementComponent.h) · [.cpp](Source/LastStand/Character/Components/LSCharacterMovementComponent.cpp), [LSRopeActionComponent.h](Source/LastStand/Character/Components/LSRopeActionComponent.h) · [.cpp](Source/LastStand/Character/Components/LSRopeActionComponent.cpp), [LSRootMotionSource.h](Source/LastStand/Character/Components/RootMotion/LSRootMotionSource.h) · [.cpp](Source/LastStand/Character/Components/RootMotion/LSRootMotionSource.cpp)

---

## 2. 보스 AI

여러 플레이어가 함께 상대하는 보스입니다. 세 가지 원칙으로 설계했습니다.

- **판단은 서버에서만** 하고, 클라이언트에는 판단 결과만 복제합니다.
- **몬스터마다 다른 점은 데이터로** 정합니다. 서브트리, 액션, 타깃 규칙, 페이즈를 `FEnemyData` 행에서 설정합니다.
- **행동 로직은 재사용 단위로** 나눕니다. StateTree는 언제 무엇을 할지만 정하고, 실제 동작은 액션 객체가 맡습니다.

<!-- 보스 교전 GIF -->

```mermaid
flowchart LR
    D[FEnemyData] -->|서브트리·액션·타깃 규칙·페이즈| P
    subgraph P[적 폰 - 서버]
        S[Perception / Stat] -->|AI.Event| H[EventHub]
        A[ActionComponent]
    end
    H --> T[StateTree<br/>Base + Linked Subtree]
    T -->|Task / Condition| A
    A -->|Multicast| V[클라 연출]
    P -->|복제 값| N[클라 애님 인스턴스]
```

### 계층과 책임

- **입력 계층** (Perception, Stat, 페이즈): 게임 상태 변화를 감지해 `AI.Event` 태그 이벤트로만 알립니다. StateTree의 존재를 알지 못합니다.
- **이벤트 허브** (`ULSAIEventHubComponent`): StateTree로 가는 유일한 통로입니다. 태그 검증과 StateTree 컴포넌트 탐색을 한곳에서 처리하므로, 발신하는 쪽은 AI 컨트롤러나 possess 시점에 의존하지 않습니다.
- **판단 계층** (StateTree): 상태 전이만 담당합니다. Global Task가 타깃과 페이즈를 출력해 바인딩 소스로 제공하고, Task와 Condition은 아래 계층을 호출하는 얇은 어댑터입니다.
- **실행 계층** (`ULSAIActionComponent` + `ULSAIActionBase`): 액션은 UObject 다형성으로 구성합니다. 활성화, 틱, 종료, 취소와 쿨다운·사거리 같은 공통 생명주기는 베이스가 처리하고, 파생 클래스(근접 스윕, 히트스캔, 미사일, 넉백, 몽타주)는 훅만 구현합니다. 파라미터는 BP 서브클래스 기본값으로 정하고, `FEnemyData::Actions`(태그와 클래스 매핑)를 읽어 적마다 인스턴스를 만듭니다.

### 액션 생명주기 규칙

StateTree와 액션 사이의 약속입니다.

- 한 번에 하나의 액션만 실행합니다. 상태 전이가 곧 액션 교체입니다.
- **완료**: 액션이 끝나면 `MakeWeakExecutionContext`로 Task를 완료합니다.
- **중단**: 상태를 벗어나면 Task가 실행 중인 액션을 취소합니다. 취소는 쿨다운을 시작하지 않고, 정상 종료와 실패 종료만 쿨다운을 시작합니다.
- **사전 검사**: `LS Can Run AI Action` Condition으로 전이 전에 실행 가능 여부를 확인합니다. 전이하면 현재 액션이 취소되는 경우에는 `bIgnoreCurrentAction`으로 대상 액션만 검사합니다.

```cpp
// 일부 생략
InstanceData.FinishedHandle = InstanceData.ActionComp->OnActionFinished.AddLambda(
    [WeakContext = Context.MakeWeakExecutionContext(), ActionTag = InstanceData.ActionTag](FGameplayTag FinishedTag, bool bSucceeded)
    {
        if (FinishedTag == ActionTag)
        {
            WeakContext.FinishTask(bSucceeded ? EStateTreeFinishTaskType::Succeeded : EStateTreeFinishTaskType::Failed);
        }
    });
```

### 서브트리 교체

- Base StateTree의 Linked Asset 상태에 `AI.Subtree.*` 태그를 지정하고, `FEnemyData::SubtreeOverrides`의 트리로 `SetLinkedStateTreeOverrides`를 호출합니다.
- 호출 순서는 `SetStateTree`, 오버라이드 적용, `StartLogic`입니다.

### 타깃 선정

- 서버에서만 동작하는 캡슐로 플레이어를 감지합니다.
- 선정 방식은 `Closest`, `TopDamage`, `Random`, `RandomLocked` 중 데이터로 고릅니다. `TopDamage`는 `ULSStatComponent`에 기록된 공격자별 누적 데미지를 사용합니다.
- `TargetChangeDelay` 동안은 타깃을 유지하고, 타깃이 사라지거나 범위를 벗어나면 바로 다시 선정합니다.
- 처음 감지하면 `Initialization`, 타깃이 바뀌면 `TargetChange` 이벤트를 보냅니다. 처음 감지할 때 감지 범위를 넓히고, 일정 시간 아무도 없으면 범위를 되돌린 뒤 `Idle` 이벤트를 보냅니다.

### 페이즈

- 체력 비율 임계치를 내림차순으로 정렬해 두고, 한 번에 여러 임계치를 넘으면 가장 낮은 것만 발동합니다.
- 현재 페이즈는 `CurrentPhase`로 복제하고 `FLSPhaseChangePayload`와 함께 이벤트로 보냅니다.

### 보조 Task와 EQS

- `LS Random Interval Event`: 상태가 켜져 있는 동안 최소~최대 사이의 랜덤 간격으로 이벤트 허브에 이벤트를 보냅니다. 일정 시간마다 패턴을 바꾸는 전이에 씁니다.
- `LS Set Move Speed`: 상태에 머무는 동안만 최대 이동 속도를 바꾸고, 나갈 때 원래 값으로 되돌립니다.
- `LS Jump To Location`: 지정 위치까지 `LaunchCharacter`로 포물선 점프하고 착지하면 완료합니다. 서버에서만 실행하고 궤적은 CMC 복제로 클라이언트에 전달됩니다.
- EQS 컨텍스트 `LS Target`: 퍼셉션 컴포넌트가 고른 현재 타깃을 쿼리 기준점으로 넘깁니다. 점프 위치나 주변 이동 위치를 고를 때 씁니다.

### 서버 판단과 클라이언트 표현의 분리

- AI 컨트롤러는 서버에만 있으므로, 판단 결과(컨트롤 회전, 턴 플래그, 페이즈, 원거리 공격 중 여부)만 복제하고 클라이언트 애님 인스턴스가 이를 해석합니다.
- 애님 인스턴스는 복제된 컨트롤 회전으로 에임 오프셋을 계산하고, 컨트롤 방향 기준 이동 각도로 타깃을 바라보며 옆으로 걷는 스트레이프를 표현합니다.
- 몸 방향은 `RInterpTo`로 컨트롤 방향을 따라가고, 차이가 60도 이상이면 서버가 턴 플래그를 설정합니다. 몽타주 재생 중에는 회전하지 않습니다.
- 액션 몽타주와 디버그 표시는 `NetMulticast`로 전달합니다.

관련 코드
- 이벤트: [LSAIEventHubComponent.cpp](Source/LastStand/AI/Components/LSAIEventHubComponent.cpp), [LSAIPerceptionComponent.cpp](Source/LastStand/AI/Components/LSAIPerceptionComponent.cpp)
- 판단: [LSStateTreeCombatGlobalTask.cpp](Source/LastStand/AI/StateTree/LSStateTreeCombatGlobalTask.cpp), [LSStateTreeRunAIActionTask.cpp](Source/LastStand/AI/StateTree/LSStateTreeRunAIActionTask.cpp), [LSStateTreeCanRunAIActionCondition.cpp](Source/LastStand/AI/StateTree/LSStateTreeCanRunAIActionCondition.cpp), [LSStateTreeRandomEventTask.cpp](Source/LastStand/AI/StateTree/LSStateTreeRandomEventTask.cpp), [LSStateTreeSetMoveSpeedTask.cpp](Source/LastStand/AI/StateTree/LSStateTreeSetMoveSpeedTask.cpp), [LSStateTreeJumpToLocationTask.cpp](Source/LastStand/AI/StateTree/LSStateTreeJumpToLocationTask.cpp), [LSEnvQueryContext_Target.cpp](Source/LastStand/AI/EQS/LSEnvQueryContext_Target.cpp), [LSAIController.cpp](Source/LastStand/AI/LSAIController.cpp)
- 실행: [LSAIActionComponent.cpp](Source/LastStand/AI/Components/LSAIActionComponent.cpp), [LSAIActionBase.h](Source/LastStand/AI/Action/LSAIActionBase.h) · [.cpp](Source/LastStand/AI/Action/LSAIActionBase.cpp)
- 데이터·표현: [LSEnemyData.h](Source/LastStand/DataTable/LSEnemyData.h), [LSEnemyBase.cpp](Source/LastStand/AI/LSEnemyBase.cpp), [LSAIAnimInstance.cpp](Source/LastStand/Animation/LSAIAnimInstance.cpp)

---

## 3. 절차적 던전 생성

방 서브레벨의 문을 서로 맞춰 이어 붙이는 생성기입니다. Runtime 모듈(`MapGenerator`)과 Editor 모듈(`MapGeneratorEditor`)로 나눈 플러그인으로 만들었습니다.

<!-- 던전 생성 GIF -->

```mermaid
flowchart LR
    A[메인 체인 DFS] --> B[사이드룸 확장]
    B --> C[레벨 스트리밍]
```

### 배치

- `BuildChain`: 출구 문과 후보 방을 섞어서 순서대로 시도하고, 배치할 수 없으면 이전 단계로 돌아가는 백트래킹 DFS입니다. `RoomCount`에 도달하면 끝 방을 배치합니다.
- `GrowSideRooms`: 메인 체인 이후 남은 문에 `SideRoomDepth`만큼 곁방을 붙입니다.
- `CanPlaceRoom`: 방 경계 박스를 월드 AABB로 변환해 기존 방과 겹치는지 검사합니다. 벽이 맞닿는 경우는 허용합니다.
- `FRandomStream(Seed)`을 사용해 같은 시드에서는 같은 결과가 나옵니다.
- 결과는 `ULevelStreamingDynamic`으로 방마다 스트리밍합니다.

### 네트워크 동기화

- 생성이 결정적이므로 배치 결과 대신 **시드 하나만 복제**합니다. 서버가 `BeginPlay`에서 시드를 정하고(`bUseRandomSeed`면 판마다 랜덤), 서버와 모든 클라이언트가 같은 시드로 각자 생성하고 스트리밍합니다.
- 방 인스턴스 이름을 배치 인덱스로 고정해(`<Room>_MapRoom_<i>`) 머신 간 스트리밍 레벨 이름을 맞춥니다. 기본 이름은 프로세스 전역 카운터라 머신마다 달라질 수 있습니다.

```cpp
const float ChildYaw = YawOfDir(OppositeDir(PWorldDir)) - YawOfDir(CDoor.Direction);
const FRotator Rot(0.f, ChildYaw, 0.f);
const FVector  T = PWorldPos - Rot.RotateVector(CDoor.Location);
return FTransform(Rot.Quaternion(), T);
```

### 에디터 툴

- 방 레벨에 `ABoxVolume`(경계)과 `ADoorVolume`(문)을 배치하고, DataTable 우클릭 메뉴로 경계와 문 정보를 행에 export합니다. 입구가 1개인지 확인합니다.
- `AMapGenerator` 액터에 생성 데이터(테이블, 방 개수, 시드, 사이드룸 깊이)를 지정하고 Generate / Clear 버튼으로 확인합니다.

관련 코드: [MapGeneratorSubsystem.h](Plugins/MapGenerator/Source/MapGenerator/Public/MapGeneratorSubsystem.h) · [.cpp](Plugins/MapGenerator/Source/MapGenerator/Private/MapGeneratorSubsystem.cpp), [MapData.h](Plugins/MapGenerator/Source/MapGenerator/Public/MapData.h), [DoorVolume.h](Plugins/MapGenerator/Source/MapGenerator/Public/DoorVolume.h), [BoxVolume.h](Plugins/MapGenerator/Source/MapGenerator/Public/BoxVolume.h), [MapGeneratorActor.h](Plugins/MapGenerator/Source/MapGenerator/Public/MapGeneratorActor.h), [MapGeneratorEditor.cpp](Plugins/MapGenerator/Source/MapGeneratorEditor/Private/MapGeneratorEditor.cpp)

---

## 4. UI 시스템

게임플레이 코드가 위젯 클래스를 직접 참조하지 않도록 만든 UI 구조입니다. 컴포넌트와 액터는 GameplayTag로 위치를 지정하거나 이벤트를 보내기만 하고, 위젯 생성과 입력 모드 전환은 UI 서브시스템이 맡습니다.

<!-- UI GIF -->

```mermaid
flowchart LR
    PC[PlayerController] -->|루트 레이아웃 생성| U[ULSUISubsystem]
    GM[GameModeInfo] -->|레이어에 위젯 추가| U
    W[무기 / 상호작용] -->|슬롯에 위젯 주입| U
    C[Stat / Equipment / Weapon] -->|Broadcast| E[ULSUIEventSubsystem]
    E -->|구독| H[HUD 위젯]
```

### 레이어와 입력 모드

- 로컬 플레이어 컨트롤러가 루트 레이아웃을 만들면, 루트 레이아웃이 Game / Menu / Modal 레이어를 `UI.Game`, `UI.Menu`, `UI.Modal` 태그로 서브시스템에 등록합니다.
- 레이어마다 올릴 위젯은 게임 모드 데이터 에셋(`ULSGameModeInfoData`)에 소프트 클래스로 적어 둡니다. 게임 스테이트의 `ULSGameModeInfoComponent`가 이 데이터를 복제받은 뒤 클라이언트에서 위젯을 만듭니다.
- 레이어는 열린 위젯의 스택을 관리합니다. 위젯을 열면 스택 맨 위로 올리고, 닫았을 때 스택이 비면 레이어도 꺼집니다.
- 레이어가 켜지거나 꺼질 때마다 Menu → Modal → Game 순으로 켜진 레이어를 찾아, 그 레이어에 설정된 입력 모드(Game / GameAndUI / UIOnly)를 적용하고 맨 위 위젯에 포커스를 줍니다. 메뉴를 닫으면 별도 처리 없이 게임 입력으로 돌아옵니다.
- 인벤토리 키는 컨트롤러가 이벤트 버스로 알리기만 하고, 인벤토리 위젯이 그 이벤트를 받아 스스로 열고 닫습니다.
- 데디 서버에서는 `ShouldCreateSubsystem`에서 UI 서브시스템을 만들지 않습니다.

### 슬롯 주입

- 조준선, 무기 정보, 주변 아이템 목록처럼 HUD 안에서 내용이 바뀌는 자리는 `ULSSlotWidget`에 `UI.Slot.*` 태그를 붙여 두고, 바깥에서 위젯 클래스만 넘겨 채웁니다.
- 무기는 장착할 때 `FWeaponData`에 지정된 조준선·무기 정보 위젯을 넣고, 해제할 때 받아 둔 `FSlotHandle`로 뺍니다. 무기를 추가해도 HUD 블루프린트는 고치지 않습니다.
- 슬롯이 아직 만들어지지 않았을 때 들어온 요청은 `PendingInjections`에 두었다가 슬롯이 등록될 때 채웁니다. 폰과 HUD 중 어느 쪽이 먼저 생성되든 결과가 같습니다.

```cpp
void ULSUISubsystem::RegisterSlot(FGameplayTag SlotTag, ULSSlotWidget* SlotWidget)
{
    SlotWidgetMap.FindOrAdd(SlotTag).Add(SlotWidget);

    // 슬롯보다 먼저 들어온 주입 요청 처리
    if (TArray<FSlotEntry>* Pending = PendingInjections.Find(SlotTag))
    {
        for (const FSlotEntry& Entry : *Pending)
        {
            SlotWidget->AddWidget(Entry.WidgetClass, Entry.HandleId);
        }
        PendingInjections.Remove(SlotTag);
    }
}
```

### 이벤트 버스

- `ULSUIEventSubsystem`은 체력, 탄약, 예비 탄약, 장비 변경, 포커스 무기 변경, 데미지, 인벤토리 입력 델리게이트만 갖고 있습니다. 컴포넌트는 브로드캐스트만 하고, 위젯은 `NativeConstruct`에서 구독하고 `NativeDestruct`에서 해제합니다.
- 체력 이벤트는 `OnRep_CurrentHealth`에서 보내되, 로컬에서 조종하는 플레이어 폰일 때만 보냅니다. 적이나 다른 플레이어의 체력이 내 HUD에 반영되지 않게 하려는 것입니다.
- 예비 탄약처럼 포커스 무기에 따라 의미가 바뀌는 값은 이벤트에 값을 싣지 않고 "바뀌었다"는 신호만 보냅니다. 위젯이 신호를 받으면 장비 컴포넌트에서 직접 다시 읽습니다. 위젯이 복제보다 늦게 만들어지는 경우를 위해 `NativeConstruct`에서도 한 번 읽습니다.

### 월드 체력바와 데미지 넘버

- 적 체력바는 위젯 컴포넌트 안에 있어서 위젯이 자기 폰을 찾기 어렵습니다. 그래서 적이 `GetUserWidgetObject()`로 위젯을 꺼내 스탯 컴포넌트를 넘겨 줍니다. 체력이 가득 차 있을 때는 숨기고, 피해를 입은 뒤부터 보입니다.
- 데미지 넘버는 정해진 개수만큼 미리 만들어 두고 재사용합니다. 쉬고 있는 위젯이 없으면 순서대로 돌아가며 가장 먼저 쓴 것부터 다시 씁니다.

### 인벤토리와 드래그 앤 드롭

- 인벤토리, 주변 아이템 목록, 장비칸 사이의 이동은 `UDragDropOperation`을 상속한 아이템/장비 오퍼레이션 두 가지로 처리합니다. 놓은 칸과 오퍼레이션 종류로 줍기·버리기·장착·해제를 구분해 서버 RPC만 보냅니다.
- UI는 결과를 직접 고치지 않습니다. 복제된 인벤토리(`FFastArraySerializer`)나 후보 목록이 바뀌면 그 델리게이트를 받아 목록을 다시 그립니다.
- 목록을 다시 그리면 드래그 중이던 엔트리 위젯이 사라지므로, 진행 중인 드래그는 취소합니다.
- 우클릭은 드래그와 같은 동작을 바로 실행합니다. 인벤토리에서는 장착, 주변 목록에서는 줍기입니다.

이 외에 컨트롤 회전의 Yaw로 눈금 띠를 옮기는 나침반 바, 장비 4칸을 보여 주고 포커스 무기가 아닌 칸은 흐리게 처리하는 장비 HUD가 있습니다.

관련 코드
- 구조: [LSUISubsystem.h](Source/LastStand/UI/LSUISubsystem.h) · [.cpp](Source/LastStand/UI/LSUISubsystem.cpp), [LSLayerWidget.cpp](Source/LastStand/UI/LSLayerWidget.cpp), [LSSlotWidget.cpp](Source/LastStand/UI/LSSlotWidget.cpp), [LSWidgetBase.cpp](Source/LastStand/UI/LSWidgetBase.cpp), [LSGameModeInfoComponent.cpp](Source/LastStand/GameState/Components/LSGameModeInfoComponent.cpp)
- 이벤트: [LSUIEventSubsystem.h](Source/LastStand/UI/LSUIEventSubsystem.h), [LSPlayerFocusEquipmentWidget.cpp](Source/LastStand/UI/Widget/LSPlayerFocusEquipmentWidget.cpp)
- 위젯: [LSDamageLayerWidget.cpp](Source/LastStand/UI/Widget/LSDamageLayerWidget.cpp), [LSEnemyStatWidget.cpp](Source/LastStand/UI/Widget/LSEnemyStatWidget.cpp), [LSInventorySlotWidget.cpp](Source/LastStand/UI/Widget/LSInventorySlotWidget.cpp), [LSInventoryEntryWidget.cpp](Source/LastStand/UI/Widget/LSInventoryEntryWidget.cpp), [LSCompassBarWidget.cpp](Source/LastStand/UI/Widget/LSCompassBarWidget.cpp)

---

## 기타 구현

- 액션 상태: 대시, 장전, 장착 상태를 GameplayTag로 모으고 `ActionRules`로 입력 차단과 취소를 설정합니다. ([LSActionStateComponent.cpp](Source/LastStand/Character/Components/LSActionStateComponent.cpp))
- 인벤토리/장비: `FFastArraySerializer` 기반 인벤토리, 배열로 복제한 뒤 클라이언트에서 다시 구성하는 장비 맵, 무기 해제 시 탄약 반환. ([LSInventoryComponent.cpp](Source/LastStand/Character/Components/LSInventoryComponent.cpp), [LSEquipmentComponent.cpp](Source/LastStand/Character/Components/LSEquipmentComponent.cpp))
- 무기 연출: 무기별 애님 레이어 링크, `FTimeline` 조준 줌, 몽타주 타입별 재생, 발사/장전 노티파이의 사운드와 Niagara. ([LSWeaponBase.cpp](Source/LastStand/Item/Equipment/Weapon/LSWeaponBase.cpp))
- 데이터: `UDeveloperSettings`, `ULSDataSubsystem`, DataTable 순으로 조회하고 에셋은 소프트 레퍼런스로 사용 시점에 로드합니다. ([LSDataSubsystem.h](Source/LastStand/DataTable/LSDataSubsystem.h))
- 인터페이스: Stat, Hitbox, Inventory, Interact, Interactable, ActionState, AIEventHub, AIAction 인터페이스로 플레이어와 적을 같은 방식으로 다룹니다. ([Interface](Source/LastStand/Interface))
- 테스트: 패키지 빌드한 데디 서버와 클라이언트 실행 스크립트(`RunServer.ps1`, `RunClient.ps1`).

## 리플리케이션 패턴

| 패턴 | 용도 | 예시 |
|---|---|---|
| `ReplicatedUsing` | 상태 전파와 클라이언트 후처리 | `CurrentHealth`, `CurrentAmmo` |
| `COND_SkipOwner` | 소유 클라이언트가 예측한 값은 다시 받지 않음 | `bIsAim` |
| SavedMove 플래그 | 이동 상태를 CMC 예측에 포함 | 달리기, 대시, 로프 점프, 스윙 |
| 커스텀 Root Motion Source | 서버가 확정한 궤적을 보정으로 클라이언트에 동기화 | `FLSRootMotionSource_ParabolicMoveTo` |
| `Reliable` Server RPC | 상태 전환 | `ServerRPCAim`, `ServerRPCReload` |
| `Unreliable` Multicast | 반복 연출 | `MulticastRPCPlayFireEffects` |
| 서버 계산 후 복제 | AI 판단 결과를 클라이언트 애님에 전달 | `CurrentControllerRotation`, `CurrentPhase` |
| `FFastArraySerializer` | 배열 델타 복제 | `FInventoryItemInfoArray` |

## 프로젝트 구조

```
LastStand/
├─ Source/LastStand/
│  ├─ Character/            플레이어, 베이스 캐릭터
│  │  └─ Components/        Stat, Equipment, Inventory, Interaction, Hitbox,
│  │     │                  ServerSideRewind, CharacterMovement, ActionState, RopeAction
│  │     └─ RootMotion/     커스텀 루트모션 소스
│  ├─ AI/                   EnemyBase, AIController
│  │  ├─ Action/            ActionBase, MeleeSweep, Hitscan, SpawnMissile, Knockback, PlayMontage
│  │  ├─ Components/        EventHub, Action, Perception
│  │  ├─ EQS/               타깃 컨텍스트
│  │  └─ StateTree/         Global Task, Task(액션 실행·랜덤 이벤트·이동 속도·점프), Condition
│  ├─ Projectile/           유도 미사일
│  ├─ Item/Equipment/Weapon/  무기
│  ├─ UI/                   UI 서브시스템, 레이어/슬롯, 위젯, 드래그 오퍼레이션
│  ├─ Animation/            애님 인스턴스, 노티파이
│  ├─ DataTable/ Data/ Settings/
│  ├─ Interface/ Tags/
│  └─ Player/ GameState/ Gamemode/ Props/ Save/
└─ Plugins/MapGenerator/    던전 생성 (Runtime, Editor)
```

## 개발 환경

- 언리얼 엔진 5 소스 빌드
- 모듈: Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG, GameplayTags, NetCore, Slate, SlateCore, AIModule, GameplayTasks, StateTreeModule, GameplayStateTreeModule, Niagara, MapGenerator, OnlineSubsystem(Private)
