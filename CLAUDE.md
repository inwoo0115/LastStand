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
