// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/LSStateTreeCombatGlobalTask.h"
#include "AI/LSEnemyBase.h"
#include "AI/Components/LSAIPerceptionComponent.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

EStateTreeRunStatus FLSStateTreeCombatGlobalTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 소유 폰(AIController→Pawn=ALSEnemyBase)과 퍼셉션 컴포넌트를 캐싱
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Perception = nullptr;
	InstanceData.Enemy = nullptr;

	if (AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		if (ALSEnemyBase* Enemy = Cast<ALSEnemyBase>(AIController->GetPawn()))
		{
			InstanceData.Enemy = Enemy;
			InstanceData.Perception = Enemy->GetPerceptionComponent();
		}
	}

	// 상태 선택 중에는 EnterState만 호출되고 Tick은 아직 안 돎 → 선택 시점 조건이 올바른 값을 보도록 즉시 계산
	UpdateOutputs(InstanceData);

	return EStateTreeRunStatus::Running;
}

void FLSStateTreeCombatGlobalTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Perception = nullptr;
	InstanceData.TargetActor = nullptr;
	InstanceData.Enemy = nullptr;
	InstanceData.CurrentPhase = 0;
}

EStateTreeRunStatus FLSStateTreeCombatGlobalTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UpdateOutputs(InstanceData);

	// Global Task가 종료되면 트리도 종료되므로 계속 Running 유지
	return EStateTreeRunStatus::Running;
}

void FLSStateTreeCombatGlobalTask::UpdateOutputs(FInstanceDataType& InstanceData)
{
	// 퍼셉션이 계산한 최근접 타깃을 Output으로 노출
	InstanceData.TargetActor = InstanceData.Perception ? InstanceData.Perception->GetTargetActor() : nullptr;

	// 현재 페이즈를 Output으로 노출 (페이즈별 상태 분기용)
	InstanceData.CurrentPhase = InstanceData.Enemy ? InstanceData.Enemy->GetCurrentPhase() : 0;
}
