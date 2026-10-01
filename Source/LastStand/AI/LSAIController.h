// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "LSAIController.generated.h"

/**
 *
 */
UCLASS()
class LASTSTAND_API ALSAIController : public AAIController
{
	GENERATED_BODY()

public:
	ALSAIController();

	virtual void Tick(float DeltaTime) override;

	class UStateTreeAIComponent* GetStateTreeComponent() const { return StateTreeComp; }

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// EnemyData의 SubtreeOverrides로 Linked Asset 서브트리 교체 (SetStateTree 이후, StartLogic 이전)
	void ApplySubtreeOverrides(class ALSEnemyBase* Enemy);

	// StateTree AI 실행 컴포넌트 (StateTreeAIComponentSchema 사용, AIController/Pawn 컨텍스트)
	UPROPERTY(VisibleAnywhere, Category = AI)
	TObjectPtr<class UStateTreeAIComponent> StateTreeComp;
};
