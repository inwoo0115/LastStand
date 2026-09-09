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

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// StateTree AI 실행 컴포넌트 (StateTreeAIComponentSchema 사용, AIController/Pawn 컨텍스트)
	UPROPERTY(VisibleAnywhere, Category = AI)
	TObjectPtr<class UStateTreeAIComponent> StateTreeComp;
};
