// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "LSAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	
public:
	ULSAnimInstance();

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSceonds) override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<class ALSPlayerCharacter> Owner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<class UCharacterMovementComponent> Movement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	uint8 bIsFalling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FVector Axis;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float Yaw;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float Pitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float Roll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	uint8 bIsAim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	uint8 bIsRun;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	uint8 bIsMontagePlaying;

	// 에임 보간 변수
	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	float AimBlendAlpha = 0.f;
};
