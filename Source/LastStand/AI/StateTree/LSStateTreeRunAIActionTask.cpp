// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/LSStateTreeRunAIActionTask.h"
#include "AI/Components/LSAIActionComponent.h"
#include "AI/Action/LSAIActionBase.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeAsyncExecutionContext.h"

FLSStateTreeRunAIActionTask::FLSStateTreeRunAIActionTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FLSStateTreeRunAIActionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.ActionComp = ULSAIActionComponent::FindActionComponent(Cast<AActor>(Context.GetOwner()));
	if (!InstanceData.ActionComp)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.ActionComp->StartAction(InstanceData.ActionTag, InstanceData.TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 시작 도중 즉시 끝난 행동은 바로 성공 처리
	if (!InstanceData.ActionComp->IsRunning())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 행동 종료 → Task 완료 (약참조 컨텍스트로 비동기 FinishTask)
	InstanceData.FinishedHandle = InstanceData.ActionComp->OnActionFinished.AddLambda(
		[WeakContext = Context.MakeWeakExecutionContext(), ActionTag = InstanceData.ActionTag](FGameplayTag FinishedTag, bool bSucceeded)
		{
			if (FinishedTag == ActionTag)
			{
				WeakContext.FinishTask(bSucceeded ? EStateTreeFinishTaskType::Succeeded : EStateTreeFinishTaskType::Failed);
			}
		});

	return EStateTreeRunStatus::Running;
}

void FLSStateTreeRunAIActionTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (ULSAIActionComponent* ActionComp = InstanceData.ActionComp)
	{
		// 구독 해제 후 중단 → 중단으로 인한 종료 통지가 이 Task로 돌아오지 않음
		ActionComp->OnActionFinished.Remove(InstanceData.FinishedHandle);

		// 전이로 상태를 벗어났는데 행동이 아직 실행 중이면 중단 (피격·사망 등)
		const ULSAIActionBase* CurrentAction = ActionComp->GetCurrentAction();
		if (CurrentAction && CurrentAction->GetActionTag() == InstanceData.ActionTag)
		{
			ActionComp->CancelCurrentAction();
		}
	}

	InstanceData.FinishedHandle.Reset();
	InstanceData.ActionComp = nullptr;
}
