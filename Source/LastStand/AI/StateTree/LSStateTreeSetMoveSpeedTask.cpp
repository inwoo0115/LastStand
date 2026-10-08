// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/LSStateTreeSetMoveSpeedTask.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

FLSStateTreeSetMoveSpeedTask::FLSStateTreeSetMoveSpeedTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FLSStateTreeSetMoveSpeedTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Movement = nullptr;

	// 소유 폰(AIController→Pawn)의 무브먼트 캐싱
	if (AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		if (ACharacter* Character = Cast<ACharacter>(AIController->GetPawn()))
		{
			InstanceData.Movement = Character->GetCharacterMovement();
		}
	}

	if (!InstanceData.Movement)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.PreviousSpeed = InstanceData.Movement->MaxWalkSpeed;
	InstanceData.Movement->MaxWalkSpeed = InstanceData.MoveSpeed;

	// 상태가 살아있는 동안 속도 유지
	return EStateTreeRunStatus::Running;
}

void FLSStateTreeSetMoveSpeedTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// 진입 전 속도로 원복 (중첩 상태도 Enter/Exit 스택 순서라 올바르게 복원)
	if (InstanceData.Movement && InstanceData.bRestoreOnExit)
	{
		InstanceData.Movement->MaxWalkSpeed = InstanceData.PreviousSpeed;
	}

	InstanceData.Movement = nullptr;
}
