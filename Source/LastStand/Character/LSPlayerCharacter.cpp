// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LSPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/LSInteractionComponent.h"
#include "Components/LSEquipmentComponent.h"
#include "Net/UnrealNetwork.h"

ALSPlayerCharacter::ALSPlayerCharacter()
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
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ALSPlayerCharacter::Crouching);
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
	ServerRPCRun();
}

void ALSPlayerCharacter::Interact(const FInputActionValue& Value)
{
	ServerRPCInteract();
}

void ALSPlayerCharacter::Crouching(const FInputActionValue& Value)
{
	ServerRPCCrouch();
}

void ALSPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 로컬 제외 나머지 클라이언트에게만 리플리케이트 (COND_SkipOwner)
	DOREPLIFETIME_CONDITION(ALSPlayerCharacter, bIsAim, COND_SkipOwner);
}

void ALSPlayerCharacter::ServerRPCInteract_Implementation()
{
	Interaction->Interact();
}

void ALSPlayerCharacter::ServerRPCCrouch_Implementation()
{
	if (bIsCrouch)
	{
		bIsCrouch = false;
		GetCharacterMovement()->MaxWalkSpeed -= 100.0f;
	}
	else
	{
		bIsCrouch = true;
		GetCharacterMovement()->MaxWalkSpeed += 100.0f;
	}
}

void ALSPlayerCharacter::OnRepIsRun()
{
	if (bIsRun)
	{
		GetCharacterMovement()->MaxWalkSpeed += 300.0f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed -= 300.0f;
	}
}

void ALSPlayerCharacter::OnRepIsCrouch()
{
	if (bIsCrouch)
	{
		GetCharacterMovement()->MaxWalkSpeed -= 100.0f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed += 100.0f;
	}
}

void ALSPlayerCharacter::ServerRPCRun_Implementation()
{
	if (bIsRun)
	{
		bIsRun = false;
		GetCharacterMovement()->MaxWalkSpeed -= 300.0f;
	}
	else
	{
		bIsRun = true;
		GetCharacterMovement()->MaxWalkSpeed += 300.0f;
	}
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
