// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/LSEnvQueryContext_Target.h"
#include "AI/LSEnemyBase.h"
#include "AI/Components/LSAIPerceptionComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "GameFramework/Controller.h"

void ULSEnvQueryContext_Target::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// 쿼리 주인은 컨트롤러 또는 폰 → 폰으로 통일
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	if (const AController* Controller = Cast<AController>(QueryOwner))
	{
		QueryOwner = Controller->GetPawn();
	}

	const ALSEnemyBase* Enemy = Cast<ALSEnemyBase>(QueryOwner);
	if (!Enemy)
	{
		return;
	}

	const ULSAIPerceptionComponent* Perception = Enemy->GetPerceptionComponent();
	AActor* Target = Perception ? Perception->GetTargetActor() : nullptr;

	if (Target)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
	}
}
