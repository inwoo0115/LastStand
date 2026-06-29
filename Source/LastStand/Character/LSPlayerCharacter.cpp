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
}

void ALSPlayerCharacter::ServerRPCInteract_Implementation()
{
	Interaction->Interact();
}

void ALSPlayerCharacter::ServerRPCCrouch_Implementation()
{
	if (IsCrouch)
	{
		IsCrouch = false;
		GetCharacterMovement()->MaxWalkSpeed -= 100.0f;
	}
	else
	{
		IsCrouch = true;
		GetCharacterMovement()->MaxWalkSpeed += 100.0f;
	}
}

void ALSPlayerCharacter::OnRepIsRun()
{
	if (IsRun)
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
	if (IsCrouch)
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
	if (IsRun)
	{
		IsRun = false;
		GetCharacterMovement()->MaxWalkSpeed -= 300.0f;
	}
	else
	{
		IsRun = true;
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

void ALSPlayerCharacter::Aim()
{
	// 카메라 옮기기
}


void ALSPlayerCharacter::Attack()
{
	Equipments->LaunchEquipment();
}


void ALSPlayerCharacter::Release()
{
	Equipments->ReleaseEquipment();
}

void ALSPlayerCharacter::AimRelease()
{
	// 카메라 원위치
}

void ALSPlayerCharacter::Reload()
{
	Equipments->Reload();
}
