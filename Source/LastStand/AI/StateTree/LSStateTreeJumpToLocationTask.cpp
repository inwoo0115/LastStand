// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/LSStateTreeJumpToLocationTask.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

FLSStateTreeJumpToLocationTask::FLSStateTreeJumpToLocationTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FLSStateTreeJumpToLocationTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Character = nullptr;

	AAIController* AIController = Cast<AAIController>(Context.GetOwner());
	ACharacter* Character = AIController ? Cast<ACharacter>(AIController->GetPawn()) : nullptr;
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;

	// 이미 공중이면 점프 불가
	if (!Movement || Movement->IsFalling())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 진행 중인 경로 추종 중단 (공중 입력 방지)
	AIController->StopMovement();

	// 목표는 바닥 위치, 액터 위치는 캡슐 중심 → 반높이만큼 보정
	const FVector Start = Character->GetActorLocation();
	const FVector End = InstanceData.TargetLocation + FVector(0.0f, 0.0f, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	FVector LaunchVelocity;
	if (!UGameplayStatics::SuggestProjectileVelocity_CustomArc(Character, LaunchVelocity, Start, End, Movement->GetGravityZ(), InstanceData.ArcParam))
	{
		return EStateTreeRunStatus::Failed;
	}

	Character->LaunchCharacter(LaunchVelocity, true, true);

	InstanceData.Character = Character;
	InstanceData.bLeftGround = false;
	InstanceData.ElapsedTime = 0.0f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FLSStateTreeJumpToLocationTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!IsValid(InstanceData.Character))
	{
		return EStateTreeRunStatus::Failed;
	}

	const bool bIsFalling = InstanceData.Character->GetCharacterMovement()->IsFalling();
	InstanceData.ElapsedTime += DeltaTime;

	if (bIsFalling)
	{
		InstanceData.bLeftGround = true;
	}
	else if (InstanceData.bLeftGround)
	{
		// 공중에 떴다가 다시 지면 → 착지 완료
		return EStateTreeRunStatus::Succeeded;
	}

	if (InstanceData.ElapsedTime >= InstanceData.MaxDuration)
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

void FLSStateTreeJumpToLocationTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 공중 궤적은 물리 이동이라 중단하지 않음 — 캐시만 정리
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Character = nullptr;
}
