// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeTaskBase.h"
#include "LSStateTreeJumpToLocationTask.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;
class ACharacter;

USTRUCT()
struct FLSStateTreeJumpToLocationTaskInstanceData
{
	GENERATED_BODY()

	// 착지 목표 위치 (바닥 기준 — EQS 결과/파라미터 바인딩)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FVector TargetLocation = FVector::ZeroVector;

	// 포물선 높이 (0.5 = 45°, 낮을수록 높은 포물선)
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.05", ClampMax = "0.95"))
	float ArcParam = 0.5f;

	// 착지 실패 안전장치 (초) — 초과 시 Failed
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.1"))
	float MaxDuration = 3.0f;

	// 런타임 캐시: 소유 폰
	UPROPERTY()
	TObjectPtr<ACharacter> Character = nullptr;

	// 런치 후 실제로 공중에 뜬 적이 있는지 (런치 직후 첫 틱은 아직 Walking일 수 있음)
	bool bLeftGround = false;

	// 런치 후 경과 시간
	float ElapsedTime = 0.0f;
};

// 지정 위치까지 포물선으로 점프(LaunchCharacter)하고 착지하면 완료되는 Task.
// 서버 AIController에서 실행 → 궤적은 CharacterMovement 복제로 클라 전파
USTRUCT(meta = (DisplayName = "LS Jump To Location", Category = "LastStand|AI"))
struct FLSStateTreeJumpToLocationTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLSStateTreeJumpToLocationTaskInstanceData;

	FLSStateTreeJumpToLocationTask();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
