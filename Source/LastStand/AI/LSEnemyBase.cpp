// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LSEnemyBase.h"
#include "Character/Components/LSStatComponent.h"
#include "Character/Components/LSHitboxComponent.h"
#include "Character/Components/LSServerSideRewindComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/Widget/LSEnemyStatWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "AI/LSAIController.h"

ALSEnemyBase::ALSEnemyBase()
{

	PrimaryActorTick.bCanEverTick = true;

	// 루트: 캡슐 컴포넌트
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(Capsule);
	Capsule->InitCapsuleSize(34.0f, 88.0f);

	// 스켈레탈 메시 (애셋/애님 클래스는 BP에서 지정). 히트박스 생성 전에 만들어 부착 대상으로 사용
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Capsule);
	// 캐릭터형 기본 배치, BP에서 조정
	Mesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
	// 데디 서버 포함 항상 포즈 평가 + 본 갱신 (렌더링 여부 무관) — SSR 정확도용
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	// 업데이트 레이트 최적화(간헐 평가/보간) 비활성 — 정확한 본 위치
	Mesh->bEnableUpdateRateOptimizations = false;
	// 히트 판정은 히트박스가 담당 → 메시 자체는 충돌 없음
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));

	// Stat
	Stat = CreateDefaultSubobject<ULSStatComponent>(TEXT("Stat"));

	// 서버 사이드 리와인드 (히트박스 히스토리 기록, 서버에서만 동작)
	ServerSideRewind = CreateDefaultSubobject<ULSServerSideRewindComponent>(TEXT("ServerSideRewind"));

	// 체력바 위젯 컴포넌트 (머리 위로 올려 몸통을 가리지 않게)
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
	// World space: 3D 씬의 일부로 렌더 → 화면 최상단에 겹쳐 그려지지 않음
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::World);

	// 부위별 히트박스 생성. 메시에 부착하고, 실제 본 소켓(Parent Socket)·정밀 배치는 BP(BP_TestEnemy)에서 마무리한다.
	auto CreateHitbox = [this](const TCHAR* Name, ELSHitboxType Type, float Multiplier,
		const FVector& RelLocation, const FVector& BoxExtent) -> ULSHitboxComponent*
	{
		ULSHitboxComponent* Hitbox = CreateDefaultSubobject<ULSHitboxComponent>(Name);
		Hitbox->SetupAttachment(Mesh);
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

}

ULSStatComponent* ALSEnemyBase::GetStatComponent()
{
	return Stat;
}

void ALSEnemyBase::ApplyDamage(int32 Damage)
{
	if (ULSStatComponent* StatComp = GetStatComponent())
	{
		StatComp->ApplyDamage(Damage);
	}
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
