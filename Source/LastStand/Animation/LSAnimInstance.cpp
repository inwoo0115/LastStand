// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LSAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ULSAnimInstance::ULSAnimInstance()
{
}

void ULSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ACharacter>(GetOwningActor());

	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void ULSAnimInstance::NativeUpdateAnimation(float DeltaSceonds)
{
	Super::NativeUpdateAnimation(DeltaSceonds);

	if (Movement)
	{
		Velocity = Owner->GetVelocity().Length();
		bIsFalling = Movement->IsFalling();
		Axis = Owner->GetActorTransform().InverseTransformVector(Owner->GetVelocity().GetSafeNormal(0.0001));
	}
}
