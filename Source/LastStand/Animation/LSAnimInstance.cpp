// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LSAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
			// 다른 로컬 클라이언트 용
			ControlRotation = Owner->GetCurrentControllerRotation();
		}

		// 캐릭터 트랜스폼(액터) 기준 상대 회전 = 조준 회전 - 액터 회전
		const FRotator RefRotation = Owner->GetActorRotation();
		const FRotator DeltaRotation = (ControlRotation - RefRotation).GetNormalized();

		Yaw = DeltaRotation.Yaw;
		Pitch = DeltaRotation.Pitch;
		Roll = DeltaRotation.Roll;

		// 에임 오프셋 보간 처리
		const float TargetAlpha = bIsAim ? 1.f : 0.f;
		const float Speed = bIsAim ? 20.f : 10.f;   // 에임 들어오는 / 나가는 속도

		AimBlendAlpha = FMath::FInterpTo(AimBlendAlpha, TargetAlpha, DeltaSceonds, Speed);

		if (FMath::IsNearlyEqual(AimBlendAlpha, TargetAlpha, KINDA_SMALL_NUMBER))
		{
			AimBlendAlpha = TargetAlpha;
		}
	}
}
