// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/LSStateTreeCanRunAIActionCondition.h"
#include "AI/Components/LSAIActionComponent.h"
#include "StateTreeExecutionContext.h"

bool FLSStateTreeCanRunAIActionCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const ULSAIActionComponent* ActionComp = ULSAIActionComponent::FindActionComponent(Cast<AActor>(Context.GetOwner()));
	const bool bCanRun = ActionComp && ActionComp->CanRunAction(InstanceData.ActionTag, InstanceData.TargetActor);

	return bCanRun ^ bInvert;
}
