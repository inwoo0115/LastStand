// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LSAIController.h"
#include "AI/LSEnemyBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/StateTreeAIComponent.h"
#include "StateTree.h"

ALSAIController::ALSAIController()
{
	// StateTree AI 컴포넌트 생성
	StateTreeComp = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComp"));
	StateTreeComp->SetStartLogicAutomatically(false);
}

void ALSAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ALSEnemyBase* Enemy = Cast<ALSEnemyBase>(InPawn);
	if (!Enemy)
	{
		return;
	}

	// StateTree가 지정되어 있으면 우선 실행, 없으면 기존 BehaviorTree로 폴백
	if (UStateTree* ST = Enemy->GetStateTree())
	{
		if (StateTreeComp)
		{
			StateTreeComp->SetStateTree(ST);
			StateTreeComp->StartLogic();
		}
	}
	else if (UBehaviorTree* BT = Enemy->GetBehaviorTree())
	{
		RunBehaviorTree(BT);
	}
}
