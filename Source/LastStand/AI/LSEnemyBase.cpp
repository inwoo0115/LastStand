// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LSEnemyBase.h"
#include "Character/Components/LSStatComponent.h"
#include "Character/Components/LSHitboxComponent.h"
#include "Character/Components/LSServerSideRewindComponent.h"
#include "AI/Components/LSAIPerceptionComponent.h"
#include "AI/Components/LSAIEventHubComponent.h"
#include "AI/Components/LSAIActionComponent.h"
#include "DataTable/LSDataSubsystem.h"
#include "Tags/LSGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/Widget/LSEnemyStatWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "AI/LSAIController.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

ALSEnemyBase::ALSEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);

	USkeletalMeshComponent* EnemyMesh = GetMesh();
	EnemyMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	EnemyMesh->bEnableUpdateRateOptimizations = false;
	EnemyMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// Stat
	Stat = CreateDefaultSubobject<ULSStatComponent>(TEXT("Stat"));

	// 서버 사이드 리와인드 (히트박스 히스토리 기록, 서버에서만 동작)
	ServerSideRewind = CreateDefaultSubobject<ULSServerSideRewindComponent>(TEXT("ServerSideRewind"));

	// AI 퍼셉션 캡슐 (플레이어 탐지·타겟 선정) — 루트 캡슐에 부착해 폰을 따라감
	Perception = CreateDefaultSubobject<ULSAIPerceptionComponent>(TEXT("Perception"));
	Perception->SetupAttachment(RootComponent);

	// AI 이벤트 허브 (StateTree 이벤트 전달 통로)
	EventHub = CreateDefaultSubobject<ULSAIEventHubComponent>(TEXT("EventHub"));

	// AI 행동 실행기
	ActionComp = CreateDefaultSubobject<ULSAIActionComponent>(TEXT("ActionComp"));

	// 체력바 위젯 컴포넌트
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
	// World space: 3D 씬의 일부로 렌더 → 화면 최상단에 겹쳐 그려지지 않음
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::World);

	auto CreateHitbox = [this](const TCHAR* Name, ELSHitboxType Type, float Multiplier,
		const FVector& RelLocation, const FVector& BoxExtent) -> ULSHitboxComponent*
	{
		ULSHitboxComponent* Hitbox = CreateDefaultSubobject<ULSHitboxComponent>(Name);
		Hitbox->SetupAttachment(GetMesh());
		Hitbox->SetupHitbox(Type, Multiplier);
		Hitbox->SetRelativeLocation(RelLocation);
		Hitbox->SetBoxExtent(BoxExtent);
		return Hitbox;
	};

	// 소켓 부착 시 본에 정렬되도록 상대 위치는 0(본 원점) 기본. Extent만 대략값 — 정밀 배치는 BP에서
	HeadHitbox     = CreateHitbox(TEXT("HeadHitbox"),     ELSHitboxType::Head,      2.0f, FVector::ZeroVector, FVector(12.0f, 12.0f, 12.0f));
	TorsoHitbox    = CreateHitbox(TEXT("TorsoHitbox"),    ELSHitboxType::Torso,     1.0f, FVector::ZeroVector, FVector(20.0f, 16.0f, 35.0f));
	LeftArmHitbox  = CreateHitbox(TEXT("LeftArmHitbox"),  ELSHitboxType::LeftArm,   0.7f, FVector::ZeroVector, FVector(10.0f, 8.0f, 30.0f));
	RightArmHitbox = CreateHitbox(TEXT("RightArmHitbox"), ELSHitboxType::RightArm,  0.7f, FVector::ZeroVector, FVector(10.0f, 8.0f, 30.0f));
	LeftLegHitbox  = CreateHitbox(TEXT("LeftLegHitbox"),  ELSHitboxType::LeftLeg,   0.7f, FVector::ZeroVector, FVector(10.0f, 10.0f, 40.0f));
	RightLegHitbox = CreateHitbox(TEXT("RightLegHitbox"), ELSHitboxType::RightLeg,  0.7f, FVector::ZeroVector, FVector(10.0f, 10.0f, 40.0f));
	WeakPointHitbox= CreateHitbox(TEXT("WeakPointHitbox"),ELSHitboxType::WeakPoint, 3.0f, FVector::ZeroVector, FVector(10.0f, 10.0f, 10.0f));

	// AI: 스폰/배치 시 커스텀 AIController가 자동 possess → OnPossess에서 BT 실행
	AIControllerClass = ALSAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 회전: 컨트롤러/이동 방향 자동 회전을 끄고 Tick에서 컨트롤 회전으로 직접 보간
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

}

ULSStatComponent* ALSEnemyBase::GetStatComponent()
{
	return Stat;
}

void ALSEnemyBase::ApplyDamage(int32 Damage, AActor* DamageCauser)
{
	if (ULSStatComponent* StatComp = GetStatComponent())
	{
		StatComp->ApplyDamage(Damage, DamageCauser);
	}
}

ULSAIEventHubComponent* ALSEnemyBase::GetAIEventHubComponent()
{
	return EventHub;
}

ULSAIActionComponent* ALSEnemyBase::GetAIActionComponent()
{
	return ActionComp;
}

void ALSEnemyBase::GetHitboxComponents(TArray<ULSHitboxComponent*>& OutHitboxes) const
{
	OutHitboxes.Reset();

	const TArray<ULSHitboxComponent*> Hitboxes = {
		HeadHitbox, TorsoHitbox, LeftArmHitbox, RightArmHitbox,
		LeftLegHitbox, RightLegHitbox, WeakPointHitbox
	};

	for (ULSHitboxComponent* Hitbox : Hitboxes)
	{
		if (Hitbox)
		{
			OutHitboxes.Add(Hitbox);
		}
	}
}

// Called when the game starts or when spawned
void ALSEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (Stat)
	{
		Stat->InitializeStatByEnemyData(EnemyName);
	}

	// 타겟 선정 방식/변경 딜레이 로드
	if (Perception)
	{
		Perception->InitializePerceptionByEnemyData(EnemyName);
	}

	// 행동 목록(EnemyData Actions) 로드 — 서버 전용
	if (ActionComp)
	{
		ActionComp->InitializeActionsByEnemyData(EnemyName);
	}

	// 체력 임계치 페이즈 전환 — 스탯 초기화 이후 구독 (초기 브로드캐스트로 발동 방지)
	InitializePhaseByEnemyData();

	// 체력바 위젯 초기화 (위젯이 생성된 머신=클라에서만. 데디 서버는 위젯 미생성)
	if (HealthBarWidget)
	{
		if (ULSEnemyStatWidget* StatUI = Cast<ULSEnemyStatWidget>(HealthBarWidget->GetUserWidgetObject()))
		{
			StatUI->InitializeWidget(Stat);
		}
	}
}

void ALSEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 컨트롤 회전 갱신 (서버에서만 컨트롤러 존재)
	if (HasAuthority() && Controller)
	{
		CurrentControllerRotation = Controller->GetControlRotation();

		// 액터 Yaw를 컨트롤 Yaw로 뒤늦게 보간 (액터 회전은 ReplicatedMovement로 클라 전파)
		const FRotator TargetRotation(0.0f, CurrentControllerRotation.Yaw, 0.0f);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, TurnInterpSpeed));

		// 턴 플래그 (서버 계산 → 복제)
		const float DeltaYaw = FRotator::NormalizeAxis(CurrentControllerRotation.Yaw - GetActorRotation().Yaw);
		bTurnRight = DeltaYaw >= TurnThresholdAngle;   // UE Yaw +는 시계방향(오른쪽)
		bTurnLeft = DeltaYaw <= -TurnThresholdAngle;
	}

	// 체력바가 각 클라의 로컬 카메라를 바라보도록 (데디 서버는 카메라 없음 → 스킵)
	if (HealthBarWidget)
	{
		if (APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			const FVector CamLoc = CamMgr->GetCameraLocation();
			const FVector WidgetLoc = HealthBarWidget->GetComponentLocation();

			// 위젯 정면이 카메라를 향하게 (수평 유지)
			FRotator LookAt = (CamLoc - WidgetLoc).Rotation();
			LookAt.Pitch = 0.0f;
			LookAt.Roll = 0.0f;
			HealthBarWidget->SetWorldRotation(LookAt);
		}
	}
}

void ALSEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALSEnemyBase, CurrentControllerRotation);
	DOREPLIFETIME(ALSEnemyBase, bTurnLeft);
	DOREPLIFETIME(ALSEnemyBase, bTurnRight);
	DOREPLIFETIME(ALSEnemyBase, bIsRangeAttacking);
	DOREPLIFETIME(ALSEnemyBase, bHasDetectedTarget);
	DOREPLIFETIME(ALSEnemyBase, CurrentPhase);
}

void ALSEnemyBase::InitializePhaseByEnemyData()
{
	// 페이즈 판정/이벤트 전송은 서버 전용
	if (!HasAuthority() || !Stat)
	{
		return;
	}

	ULSDataSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSDataSubsystem>();
	const FEnemyData* Data = Sub ? Sub->FindEnemy(EnemyName) : nullptr;
	if (!Data || Data->HealthThresholdEvents.IsEmpty())
	{
		return;
	}

	// 높은 비율부터 순서대로 통과하도록 내림차순 정렬
	HealthThresholdEvents = Data->HealthThresholdEvents;
	HealthThresholdEvents.Sort([](const FLSHealthThresholdEvent& A, const FLSHealthThresholdEvent& B)
	{
		return A.HealthRatio > B.HealthRatio;
	});
	NextHealthThresholdIndex = 0;

	Stat->OnHealthChanged.AddUObject(this, &ALSEnemyBase::HandleHealthChanged);
}

void ALSEnemyBase::HandleHealthChanged(int32 NewCurrentHealth, int32 NewMaxHealth)
{
	// 사망(0)은 Death 흐름이 처리
	if (!HasAuthority() || NewMaxHealth <= 0 || NewCurrentHealth <= 0)
	{
		return;
	}

	const float Ratio = static_cast<float>(NewCurrentHealth) / static_cast<float>(NewMaxHealth);

	// 이번 피격으로 넘은 임계치 중 가장 낮은 것 하나만 발동 (나머지는 소비)
	int32 CrossedIndex = INDEX_NONE;
	while (HealthThresholdEvents.IsValidIndex(NextHealthThresholdIndex)
		&& Ratio <= HealthThresholdEvents[NextHealthThresholdIndex].HealthRatio)
	{
		CrossedIndex = NextHealthThresholdIndex;
		++NextHealthThresholdIndex;
	}

	if (CrossedIndex == INDEX_NONE)
	{
		return;
	}

	const FLSHealthThresholdEvent& Threshold = HealthThresholdEvents[CrossedIndex];
	CurrentPhase = Threshold.Phase;

	if (EventHub)
	{
		FLSPhaseChangePayload Payload;
		Payload.Phase = CurrentPhase;

		const FGameplayTag EventTag = Threshold.EventTag.IsValid() ? Threshold.EventTag : LSAITags::Event_PhaseChange;
		EventHub->SendEvent(EventTag, FConstStructView::Make(Payload));
	}
}

void ALSEnemyBase::SetHasDetectedTarget(bool bInHasDetectedTarget)
{
	if (HasAuthority())
	{
		bHasDetectedTarget = bInHasDetectedTarget;
	}
}

void ALSEnemyBase::SetIsRangeAttacking(bool bInIsRangeAttacking)
{
	if (HasAuthority())
	{
		bIsRangeAttacking = bInIsRangeAttacking;
	}
}
