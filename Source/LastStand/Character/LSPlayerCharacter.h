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

	void Interact(const FInputActionValue& Value);

	void Crouching(const FInputActionValue& Value);

	void Jump();

	void Attack();

	void Aim();

	void Release();

	void AimRelease();

	void Reload();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ReloadAction;

	// RPC
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerRPCRun();

	UFUNCTION(Server, Reliable)
	void ServerRPCInteract();

	UFUNCTION(Server, Reliable)
	void ServerRPCCrouch();

	// Character State flag
	UPROPERTY(ReplicatedUsing = OnRepIsRun)
	bool IsRun = false;

	// Character State flag
	UPROPERTY(ReplicatedUsing = OnRepIsCrouch)
	bool IsCrouch = false;

	UFUNCTION()
	void OnRepIsRun();

	UFUNCTION()
	void OnRepIsCrouch();
};
