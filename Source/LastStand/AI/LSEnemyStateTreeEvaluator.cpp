// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LSEnemyStateTreeEvaluator.h"
#include "AI/LSEnemyBase.h"
#include "AI/Components/LSAIPerceptionComponent.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

void FLSEnemyStateTreeEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	// 소유 폰(AIController→Pawn=ALSEnemyBase)의 퍼셉션 컴포넌트를 캐싱
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Perception = nullptr;

	if (AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		if (ALSEnemyBase* Enemy = Cast<ALSEnemyBase>(AIController->GetPawn()))
		{
			InstanceData.Perception = Enemy->GetPerceptionComponent();
		}
	}
}

void FLSEnemyStateTreeEvaluator::TreeStop(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Perception = nullptr;
	InstanceData.TargetActor = nullptr;
}

void FLSEnemyStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// 퍼셉션이 계산한 최근접 타깃을 Output으로 노출
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.TargetActor = InstanceData.Perception ? InstanceData.Perception->GetTargetActor() : nullptr;
}
