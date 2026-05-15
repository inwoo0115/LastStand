// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALSPlayerController();

	void BeginPlay() override;

	void Tick(float DeltaSeconds) override;

	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(EditAnywhere, Category = Layout, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ULSRootLayoutWidget> LayoutClass;
	
	
	
};
