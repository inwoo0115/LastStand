// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/LSStateTreeRandomEventTask.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "AI/Components/LSAIEventHubComponent.h"
#include "Interface/LSAIEventHubInterface.h"

FLSStateTreeRandomEventTask::FLSStateTreeRandomEventTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FLSStateTreeRandomEventTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.EventHub = nullptr;

	// 소유 폰(AIController→Pawn)의 이벤트 허브 캐싱
	AAIController* AIController = Cast<AAIController>(Context.GetOwner());
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (Pawn && Pawn->Implements<ULSAIEventHubInterface>())
	{
		InstanceData.EventHub = Cast<ILSAIEventHubInterface>(Pawn)->GetAIEventHubComponent();
	}

	if (!InstanceData.EventHub || !InstanceData.EventTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.RemainingTime = PickInterval(InstanceData);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FLSStateTreeRandomEventTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!IsValid(InstanceData.EventHub))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.RemainingTime -= DeltaTime;
	if (InstanceData.RemainingTime <= 0.0f)
	{
		// 시간 도달 → 이벤트 발신 후 다음 랜덤 간격 재설정
		InstanceData.EventHub->SendEvent(InstanceData.EventTag, FConstStructView(), TEXT("RandomEventTask"));
		InstanceData.RemainingTime = PickInterval(InstanceData);
	}

	return EStateTreeRunStatus::Running;
}

void FLSStateTreeRandomEventTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.EventHub = nullptr;
}

float FLSStateTreeRandomEventTask::PickInterval(const FInstanceDataType& InstanceData)
{
	const float Min = FMath::Min(InstanceData.MinInterval, InstanceData.MaxInterval);
	const float Max = FMath::Max(InstanceData.MinInterval, InstanceData.MaxInterval);
	return FMath::FRandRange(Min, Max);
}
