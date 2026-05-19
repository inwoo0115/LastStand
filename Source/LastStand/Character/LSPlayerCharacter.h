// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LSCharacterBase.h"
#include "InputActionValue.h"
#include "LSPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSPlayerCharacter : public ALSCharacterBase
{
	GENERATED_BODY()
	
public:
	ALSPlayerCharacter();

	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Walk(const FInputActionValue& Value);

	void Jump();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> WalkAction;


	// RPC
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerRPCRun();

	// Character State flag
	UPROPERTY(ReplicatedUsing = OnRepIsRun)
	bool IsRun = false;

	UFUNCTION()
	void OnRepIsRun();
};
