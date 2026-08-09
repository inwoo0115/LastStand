// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LSEnemyBase.h"
#include "Character/Components/LSStatComponent.h"
#include "Character/Components/LSHitboxComponent.h"
#include "Character/Components/LSServerSideRewindComponent.h"
#include "Components/CapsuleComponent.h"
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

	// 부위별 히트박스 생성. 여기서는 루트 캡슐에 대략적인 위치/크기로 부착만 하고,
	// 실제 본 소켓 재부착·정밀 배치는 BP(BP_TestEnemy)에서 마무리한다.
	auto CreateHitbox = [this](const TCHAR* Name, ELSHitboxType Type, float Multiplier,
		const FVector& RelLocation, const FVector& BoxExtent) -> ULSHitboxComponent*
	{
		ULSHitboxComponent* Hitbox = CreateDefaultSubobject<ULSHitboxComponent>(Name);
		Hitbox->SetupAttachment(RootComponent);
		Hitbox->SetupHitbox(Type, Multiplier);
		Hitbox->SetRelativeLocation(RelLocation);
		Hitbox->SetBoxExtent(BoxExtent);
		return Hitbox;
	};

	HeadHitbox     = CreateHitbox(TEXT("HeadHitbox"),     ELSHitboxType::Head,      2.0f, FVector(0.0f, 0.0f, 70.0f),   FVector(12.0f, 12.0f, 12.0f));
	TorsoHitbox    = CreateHitbox(TEXT("TorsoHitbox"),    ELSHitboxType::Torso,     1.0f, FVector(0.0f, 0.0f, 20.0f),   FVector(20.0f, 16.0f, 35.0f));
	LeftArmHitbox  = CreateHitbox(TEXT("LeftArmHitbox"),  ELSHitboxType::LeftArm,   0.7f, FVector(0.0f, -25.0f, 20.0f), FVector(10.0f, 8.0f, 30.0f));
	RightArmHitbox = CreateHitbox(TEXT("RightArmHitbox"), ELSHitboxType::RightArm,  0.7f, FVector(0.0f, 25.0f, 20.0f),  FVector(10.0f, 8.0f, 30.0f));
	LeftLegHitbox  = CreateHitbox(TEXT("LeftLegHitbox"),  ELSHitboxType::LeftLeg,   0.7f, FVector(0.0f, -10.0f, -45.0f),FVector(10.0f, 10.0f, 40.0f));
	RightLegHitbox = CreateHitbox(TEXT("RightLegHitbox"), ELSHitboxType::RightLeg,  0.7f, FVector(0.0f, 10.0f, -45.0f), FVector(10.0f, 10.0f, 40.0f));
	WeakPointHitbox= CreateHitbox(TEXT("WeakPointHitbox"),ELSHitboxType::WeakPoint, 3.0f, FVector(0.0f, 0.0f, 40.0f),   FVector(10.0f, 10.0f, 10.0f));

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
