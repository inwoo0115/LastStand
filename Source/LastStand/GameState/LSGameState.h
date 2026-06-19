// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "LSGameState.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ALSGameState();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class ULSGameModeInfoComponent> InfoComp;
	
};
