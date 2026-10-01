// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LSAIController.h"
#include "AI/LSEnemyBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/StateTreeAIComponent.h"
#include "StateTree.h"
#include "AI/Components/LSAIPerceptionComponent.h"
#include "DataTable/LSDataSubsystem.h"
#include "DataTable/LSEnemyData.h"
#include "StateTreeReference.h"

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
			// 스키마 검증이 설정된 StateTree 기준이라 SetStateTree 이후에 적용
			ApplySubtreeOverrides(Enemy);
			StateTreeComp->StartLogic();
		}
	}
	else if (UBehaviorTree* BT = Enemy->GetBehaviorTree())
	{
		RunBehaviorTree(BT);
	}
}

void ALSAIController::ApplySubtreeOverrides(ALSEnemyBase* Enemy)
{
	if (!Enemy || !StateTreeComp)
	{
		return;
	}

	ULSDataSubsystem* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULSDataSubsystem>() : nullptr;
	if (!Sub)
	{
		return;
	}

	const FEnemyData* Data = Sub->FindEnemy(Enemy->GetEnemyName());
	if (!Data)
	{
		return;
	}

	// 슬롯 태그 → 서브트리 매핑을 모아 한 번에 교체 (재possess 시 이전 오버라이드 잔존 방지)
	FStateTreeReferenceOverrides Overrides;
	for (const TPair<FGameplayTag, TSoftObjectPtr<UStateTree>>& Pair : Data->SubtreeOverrides)
	{
		UStateTree* Subtree = Pair.Value.LoadSynchronous();
		if (!Pair.Key.IsValid() || !Subtree)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] SubtreeOverrides: invalid slot '%s' or subtree, skipped"), *Enemy->GetEnemyName().ToString(), *Pair.Key.ToString());
			continue;
		}

		// SetStateTree가 서브트리 기본 파라미터를 동기화
		FStateTreeReference Ref;
		Ref.SetStateTree(Subtree);
		Overrides.AddOverride(FStateTreeReferenceOverrideItem(Pair.Key, MoveTemp(Ref)));
	}

	StateTreeComp->SetLinkedStateTreeOverrides(MoveTemp(Overrides));
}
