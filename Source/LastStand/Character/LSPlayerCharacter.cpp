// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LSPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/LSInteractionComponent.h"
#include "Components/LSEquipmentComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimMontage.h"

ALSPlayerCharacter::ALSPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ALSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ALSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	auto EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	// Bind Action
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::Jump);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALSPlayerCharacter::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALSPlayerCharacter::Look);
	EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::Walk);
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::Interact);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::Dash);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::Attack);
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::Aim);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ALSPlayerCharacter::Release);
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ALSPlayerCharacter::AimRelease);
	EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::Reload);
	EnhancedInputComponent->BindAction(SelectMainAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::SelectMainWeapon);
	EnhancedInputComponent->BindAction(SelectSubAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::SelectSubWeapon);

}

bool ALSPlayerCharacter::GetIsAim()
{
	return bIsAim;
}

bool ALSPlayerCharacter::GetIsRun()
{
	return bIsRun;
}

void ALSPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D Movement = Value.Get<FVector2D>();

	FRotator Rotation = GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardVector, Movement.X);
	AddMovementInput(RightVector, Movement.Y);
}

void ALSPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D Movement = Value.Get<FVector2D>();


	AddControllerPitchInput(Movement.Y * -1);
	AddControllerYawInput(Movement.X);
}

void ALSPlayerCharacter::Walk(const FInputActionValue& Value)
{
	// 로컬 즉시 토글 → CMC 플래그로 이동 속도 예측, 서버엔 애님용 상태 전달
	bIsRun = !bIsRun;
	GetLSCharacterMovement()->SetWantsToRun(bIsRun);
	ServerRPCRun(bIsRun);
}

void ALSPlayerCharacter::Interact(const FInputActionValue& Value)
{
	ServerRPCInteract();
}

void ALSPlayerCharacter::Dash()
{
	ULSCharacterMovementComponent* CMC = GetLSCharacterMovement();
	if (!CMC || !CMC->CanDash())   // 지상/입력 없음/대시 중/쿨다운
	{
		return;
	}

	// 해당 방향 대시 몽타주가 지정돼 있어야 대시 가능
	const ELSDashDirection Direction = CMC->GetDashDirectionType(CMC->GetDashDirection());
	const TSoftObjectPtr<UAnimMontage>* Found = DashMontages.Find(Direction);
	if (!Found || Found->IsNull())
	{
		return;
	}

	// 반응성: 로컬 즉시 몽타주 → 다음 이동 틱에 CMC가 루트모션 적용(서버 재현)
	PlayDashMontage(Direction);
	CMC->RequestDash();
}

void ALSPlayerCharacter::PlayDashMontage(ELSDashDirection Direction)
{
	if (const TSoftObjectPtr<UAnimMontage>* Found = DashMontages.Find(Direction))
	{
		if (UAnimMontage* Montage = Found->LoadSynchronous())
		{
			PlayAnimMontage(Montage);
		}
	}
}

void ALSPlayerCharacter::MulticastRPCPlayDashMontage_Implementation(ELSDashDirection Direction)
{
	// 소유 클라는 입력 시 이미 재생 → 중복 방지
	if (IsLocallyControlled())
	{
		return;
	}

	PlayDashMontage(Direction);
}

void ALSPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 로컬 제외 나머지 클라이언트에게만 리플리케이트 (COND_SkipOwner)
	DOREPLIFETIME_CONDITION(ALSPlayerCharacter, bIsAim, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ALSPlayerCharacter, bIsRun, COND_SkipOwner);
}

void ALSPlayerCharacter::ServerRPCInteract_Implementation()
{
	Interaction->Interact();
}

void ALSPlayerCharacter::ServerRPCRun_Implementation(bool bNewRun)
{
	// 이동 속도는 CMC가 처리 → 여기선 타 클라 애님용 상태만
	bIsRun = bNewRun;
}

void ALSPlayerCharacter::Jump()
{
	if (IsLocallyControlled())
	{
		Super::Jump();
	}
}

void ALSPlayerCharacter::OnAimRemainTimeout()
{
	if (!bIsAimActionActive)
	{
		bIsAim = false;
		ServerRPCAim(false);
	}
}

void ALSPlayerCharacter::Aim()
{
	Equipments->Aim();
	bIsAim = true;
	bIsAimActionActive = true;
	ServerRPCAim(true);
}


void ALSPlayerCharacter::Attack()
{
	Equipments->LaunchEquipment();

	if (!bIsAimActionActive)
	{
		bIsAim = true;
		ServerRPCAim(true);
		GetWorldTimerManager().SetTimer(AimRemainTimerHandle, this, &ALSPlayerCharacter::OnAimRemainTimeout, AimRemainTime, false);
	}
}


void ALSPlayerCharacter::Release()
{
	Equipments->ReleaseEquipment();
}

void ALSPlayerCharacter::AimRelease()
{
	Equipments->AimRelease();
	bIsAim = false;
	bIsAimActionActive = false;
	ServerRPCAim(false);
}

void ALSPlayerCharacter::ServerRPCAim_Implementation(bool bNewAim)
{
	// 서버 변수 변경 → COND_SkipOwner로 소유 클라 제외 나머지에 리플리케이트
	bIsAim = bNewAim;
}

void ALSPlayerCharacter::Reload()
{
	Equipments->Reload();
}

void ALSPlayerCharacter::SelectMainWeapon(const FInputActionValue& Value)
{
	ServerRPCFocusMainWeapon();
}

void ALSPlayerCharacter::SelectSubWeapon(const FInputActionValue& Value)
{
	ServerRPCFocusSubWeapon();
}

void ALSPlayerCharacter::ServerRPCFocusMainWeapon_Implementation()
{
	Equipments->FocusEquipmentByType(EEquipmentType::Main);
}

void ALSPlayerCharacter::ServerRPCFocusSubWeapon_Implementation()
{
	Equipments->FocusEquipmentByType(EEquipmentType::Sub);
}
