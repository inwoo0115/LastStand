// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LSAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ArrowComponent.h"
#include "Character/LSPlayerCharacter.h"

ULSAnimInstance::ULSAnimInstance()
{
}

void ULSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ALSPlayerCharacter>(GetOwningActor());

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
		bIsMontagePlaying = Montage_IsPlaying(nullptr);
		bIsAim = Owner->GetIsAim();
		bIsRun = Owner->GetIsRun();

		// 조준(컨트롤) 회전 가져오기
		FRotator ControlRotation;
		if (Owner->GetController())
		{
			ControlRotation = Owner->GetController()->GetControlRotation();
		}
		else
		{
			ControlRotation = Owner->GetCurrentControllerRotation();
		}

		// 캐릭터 트랜스폼(액터) 기준 상대 회전 = 조준 회전 - 액터 회전
		const FRotator RefRotation = Owner->GetArrowComponent()->GetComponentRotation();
		const FRotator DeltaRotation = (ControlRotation - RefRotation).GetNormalized();

		Yaw = DeltaRotation.Yaw;
		Pitch = DeltaRotation.Pitch;
		Roll = DeltaRotation.Roll;

	}
}
