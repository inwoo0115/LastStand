// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeTaskBase.h"
#include "LSStateTreeSetMoveSpeedTask.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;
class UCharacterMovementComponent;

USTRUCT()
struct FLSStateTreeSetMoveSpeedTaskInstanceData
{
	GENERATED_BODY()

	// 상태 진입 시 적용할 최대 이동 속도
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float MoveSpeed = 300.0f;

	// 상태 이탈 시 이전 속도로 원복할지
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bRestoreOnExit = true;

	// 런타임 캐시: 소유 폰의 무브먼트 컴포넌트
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> Movement = nullptr;

	// 원복용 진입 전 속도
	float PreviousSpeed = 0.0f;
};

// 상태가 활성인 동안 소유 폰의 MaxWalkSpeed를 변경하는 Task.
// 다른 Task와 병렬로 붙여 쓰며, 상태 이탈 시 이전 속도로 원복
USTRUCT(meta = (DisplayName = "LS Set Move Speed", Category = "LastStand|AI"))
struct FLSStateTreeSetMoveSpeedTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLSStateTreeSetMoveSpeedTaskInstanceData;

	FLSStateTreeSetMoveSpeedTask();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
