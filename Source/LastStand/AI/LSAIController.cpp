// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LSAIController.h"
#include "AI/LSEnemyBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/StateTreeAIComponent.h"
#include "StateTree.h"
#include "AI/Components/LSAIPerceptionComponent.h"

ALSAIController::ALSAIController()
{
	// StateTree AI 컴포넌트 생성
	StateTreeComp = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComp"));
	StateTreeComp->SetStartLogicAutomatically(false);
}

void ALSAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO: 테스트용 — 퍼셉션 타깃을 포커스로 지정해 컨트롤 회전이 타깃을 향하게 함
	if (ALSEnemyBase* Enemy = Cast<ALSEnemyBase>(GetPawn()))
	{
		if (ULSAIPerceptionComponent* Perception = Enemy->GetPerceptionComponent())
		{
			if (AActor* Target = Perception->GetTargetActor())
			{
				SetFocus(Target);
			}
			else
			{
				ClearFocus(EAIFocusPriority::Gameplay);
			}
		}
	}
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
