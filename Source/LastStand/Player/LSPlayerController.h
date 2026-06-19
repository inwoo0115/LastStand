// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
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

	virtual void OnPossess(APawn* InPawn) override;

	virtual void SetupInputComponent() override;

	void ToggleInventory(const FInputActionValue& Value);

private:
	UPROPERTY(EditAnywhere, Category = Layout, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ULSRootLayoutWidget> LayoutClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InventoryAction;

	UPROPERTY()
	TObjectPtr<class ULSUIEventSubsystem> CachedUISubsystem;
};
