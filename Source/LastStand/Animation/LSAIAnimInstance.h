// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "LSAIAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSAIAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	ULSAIAnimInstance();

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSceonds) override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<class ALSEnemyBase> Owner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<class UCharacterMovementComponent> Movement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	uint8 bIsFalling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FVector Axis;

	// AI 컨트롤 회전(Yaw) 기준 이동 방향 단위 벡터 (스트레이프 블렌드스페이스 입력)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FVector ControlAxis;

	// 컨트롤 회전이 액터보다 왼쪽/오른쪽으로 임계각 이상 차이 (턴 애니메이션용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	uint8 bTurnLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	uint8 bTurnRight;

	// 원거리 공격(Hitscan 액션) 실행 중 (복제 플래그)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	uint8 bIsRangeAttacking;

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
