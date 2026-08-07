// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LSAIController.h"
#include "AI/LSEnemyBase.h"
#include "BehaviorTree/BehaviorTree.h"

void ALSAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ALSEnemyBase* Enemy = Cast<ALSEnemyBase>(InPawn))
	{
		if (UBehaviorTree* BT = Enemy->GetBehaviorTree())
		{
			// BT에 지정된 Blackboard를 자동 사용
			RunBehaviorTree(BT);
		}
	}
}
