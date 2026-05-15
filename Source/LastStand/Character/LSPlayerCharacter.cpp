// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LSPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
}

void ALSPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ALSPlayerCharacter::Jump()
{
	if (IsLocallyControlled())
	{
		Super::Jump();
	}
}