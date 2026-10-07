// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LSAIAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/LSEnemyBase.h"
#include "LSAIAnimInstance.h"

ULSAIAnimInstance::ULSAIAnimInstance()
{
}


void ULSAIAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ALSEnemyBase>(GetOwningActor());

	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void ULSAIAnimInstance::NativeUpdateAnimation(float DeltaSceonds)
{
	Super::NativeUpdateAnimation(DeltaSceonds);

	if (Movement)
	{
		Velocity = Owner->GetVelocity().Length();
		bIsFalling = Movement->IsFalling();
		Axis = Owner->GetActorTransform().InverseTransformVector(Owner->GetVelocity().GetSafeNormal(0.0001));
		bIsMontagePlaying = Montage_IsPlaying(nullptr);
		bTurnLeft = Owner->GetTurnLeft();
		bTurnRight = Owner->GetTurnRight();
		bIsRangeAttacking = Owner->GetIsRangeAttacking();

		// 조준(컨트롤) 회전 가져오기
		FRotator ControlRotation;
		if (Owner->GetController())
		{
			ControlRotation = Owner->GetController()->GetControlRotation();
		}
		else
		{
			// 클라이언트: AI 컨트롤러가 없으므로 복제된 컨트롤 회전 사용
			ControlRotation = Owner->GetCurrentControllerRotation();
		}

		// 컨트롤 회전(Yaw) 기준 이동 방향 — 피치 제외해 전후 성분 유지
		const FRotator ControlYaw(0.0f, ControlRotation.Yaw, 0.0f);
		ControlAxis = ControlYaw.UnrotateVector(Owner->GetVelocity().GetSafeNormal(0.0001));

		// 캐릭터 트랜스폼(액터) 기준 상대 회전 = 조준 회전 - 액터 회전
		const FRotator RefRotation = Owner->GetActorRotation();
		const FRotator DeltaRotation = (ControlRotation - RefRotation).GetNormalized();

		Yaw = DeltaRotation.Yaw;
		Pitch = DeltaRotation.Pitch;
		Roll = DeltaRotation.Roll;

	}
}