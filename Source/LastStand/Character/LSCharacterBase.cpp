// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LSCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Data/LSCharacterControlData.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "Components/LSEquipmentComponent.h"
#include "Components/LSInventoryComponent.h"
#include "Components/LSInteractionComponent.h"
#include "Components/LSStatComponent.h"
#include "GameState/LSGameState.h"


// Sets default values
ALSCharacterBase::ALSCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 무브먼트 컴포넌트
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 400.0f;
	GetCharacterMovement()->bEnablePhysicsInteraction = false;

	// 캡슐 컴포넌트
	GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);

	// Camera, Spring Arm 컴포넌트
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Inventory
	Inventory = CreateDefaultSubobject<ULSInventoryComponent>(TEXT("Inventory"));
	
	//  Equipments
	Equipments = CreateDefaultSubobject<ULSEquipmentComponent>(TEXT("Equipments"));

	// Interaction
	Interaction = CreateDefaultSubobject<ULSInteractionComponent>(TEXT("Interaction"));

	// Stat
	Stat = CreateDefaultSubobject<ULSStatComponent>(TEXT("Stat"));
}

ULSInteractionComponent* ALSCharacterBase::GetInteractionComponent()
{
	return Interaction;
}

ULSInventoryComponent* ALSCharacterBase::GetInventoryComponent()
{
	return Inventory;
}

ULSStatComponent* ALSCharacterBase::GetStatComponent()
{
	return Stat;
}

void ALSCharacterBase::ApplyDamage(int32 Damage)
{
	if (ULSStatComponent* StatComp = GetStatComponent())
	{
		StatComp->ApplyDamage(Damage);
	}
}

USpringArmComponent* ALSCharacterBase::GetSpringArmComponent()
{
	return SpringArm;
}

FTransform ALSCharacterBase::GetCurrentCameraTransform() const
{
	if (!SpringArm)
	{
		return FTransform::Identity;
	}

	return SpringArm->GetSocketTransform(USpringArmComponent::SocketName);
}

float ALSCharacterBase::GetCurrentSpringArmLength() const
{
	if (!SpringArm)
	{
		return 0.0f;
	}

	return SpringArm->TargetArmLength;
}

FRotator ALSCharacterBase::GetCurrentControllerRotation() const
{
	return CurrentControllerRotation;
}

void ALSCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 컨트롤러 로테이션 업데이트
	if (HasAuthority())
	{
		CurrentControllerRotation = Controller->GetControlRotation();
	}
}

// Called when the game starts or when spawned
void ALSCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// 컨트롤 데이터 설정: GameMode InfoData의 ControlData 우선, 없으면 캐릭터 자체 맵 기본값 폴백
	ULSCharacterControlData* GameControlData = nullptr;
	if (ALSGameState* GS = GetWorld()->GetGameState<ALSGameState>())
	{
		GameControlData = GS->GetControlData();
	}

	if (GameControlData)
	{
		SetCharacterControlData(GameControlData);
	}
	else
	{
		SetCharacterControlData(CurrentCharacterControlType);
	}
}

void ALSCharacterBase::SetCharacterControlData(ECharacterControlType ControlType)
{
	ULSCharacterControlData* Data = CharacterControlManager.FindRef(ControlType);
	SetCharacterControlData(Data);
}

void ALSCharacterBase::SetCharacterControlData(ULSCharacterControlData* CharacterControlData)
{
	if (!CharacterControlData)
	{
		return;
	}

	// Character Control Setting

	// pawn
	bUseControllerRotationYaw = CharacterControlData->bUseControllerRotationYaw;

	// CharacterMovement
	GetCharacterMovement()->bOrientRotationToMovement = CharacterControlData->bOrientRotationToMovement;
	GetCharacterMovement()->bUseControllerDesiredRotation = CharacterControlData->bUseControllerDesiredRotation;
	GetCharacterMovement()->RotationRate = CharacterControlData->RotationRate;

	// SpringArm
	SpringArm->TargetArmLength = CharacterControlData->TargetArmLength;
	SpringArm->SetRelativeLocation(CharacterControlData->RelativeLocation);
	SpringArm->bUsePawnControlRotation = CharacterControlData->bUsePawnControlRotation;
	SpringArm->bInheritPitch = CharacterControlData->bInheritPitch;
	SpringArm->bInheritRoll = CharacterControlData->bInheritRoll;
	SpringArm->bInheritYaw = CharacterControlData->bInheritYaw;
	SpringArm->bDoCollisionTest = CharacterControlData->bDoCollisionTest;

	// AnimInstance
	GetMesh()->SetAnimInstanceClass(CharacterControlData->AnimBlueprintClass);

	// Input System mapping
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		if (auto* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (auto* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				SubSystem->ClearAllMappings();
				SubSystem->AddMappingContext(CharacterControlData->InputMappingContext, 0);
			}
		}
	}
}

void ALSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALSCharacterBase, CurrentCharacterControlType);
	DOREPLIFETIME(ALSCharacterBase, CurrentControllerRotation);
}

void ALSCharacterBase::OnRep_ControlType()
{
	SetCharacterControlData(CurrentCharacterControlType);
}

