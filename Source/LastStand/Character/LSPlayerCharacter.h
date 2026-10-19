// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LSCharacterBase.h"
#include "InputActionValue.h"
#include "Character/Components/LSCharacterMovementComponent.h"
#include "LSPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSPlayerCharacter : public ALSCharacterBase
{
	GENERATED_BODY()
	
public:
	ALSPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	bool GetIsAim();

	bool GetIsRun();

	// 서버(CMC PerformDash) → 타 클라 대시 몽타주. 소유 클라는 로컬 재생했으므로 스킵
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCPlayDashMontage(ELSDashDirection Direction);

protected:
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Walk(const FInputActionValue& Value);

	void Interact(const FInputActionValue& Value);

	void Dash();

	// 방향별 대시 몽타주 재생 (캐릭터 메시)
	void PlayDashMontage(ELSDashDirection Direction);

	void Jump();

	void Attack();

	void Aim();

	void Release();

	void AimRelease();

	void Reload();

	void SelectMainWeapon(const FInputActionValue& Value);

	void SelectSubWeapon(const FInputActionValue& Value);

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
	TObjectPtr<class UInputAction> DashAction;

	// 방향별 대시 몽타주 (이동은 CMC 루트모션 소스가 담당 → 애님 루트모션 끄기)
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	TMap<ELSDashDirection, TSoftObjectPtr<class UAnimMontage>> DashMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SelectMainAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SelectSubAction;

	// RPC
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 토글 결과를 서버에 전달 (애님용 bIsRun 동기화, 이동 속도는 CMC 플래그)
	UFUNCTION(Server, Reliable)
	void ServerRPCRun(bool bNewRun);

	UFUNCTION(Server, Reliable)
	void ServerRPCInteract();

	UFUNCTION(Server, Reliable)
	void ServerRPCAim(bool bNewAim);

	UFUNCTION(Server, Reliable)
	void ServerRPCFocusMainWeapon();

	UFUNCTION(Server, Reliable)
	void ServerRPCFocusSubWeapon();

	// Character State flag (소유 클라는 로컬 세팅 → COND_SkipOwner)
	UPROPERTY(Replicated)
	bool bIsRun = false;

	UPROPERTY(Replicated)
	bool bIsAim = false;

	// Aim Timer
	UPROPERTY(EditAnywhere, Category = Aim, meta = (AllowPrivateAccess = "true"))
	float AimRemainTime = 3.0f;

	FTimerHandle AimRemainTimerHandle;

	bool bIsAimActionActive = false;

	void OnAimRemainTimeout();
};
