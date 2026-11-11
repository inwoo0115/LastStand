// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "Tags/LSGameplayTags.h"
#include "LSStateTreeRandomEventTask.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;
class ULSAIEventHubComponent;

USTRUCT()
struct FLSStateTreeRandomEventTaskInstanceData
{
	GENERATED_BODY()

	// 보낼 이벤트 태그 (AI.Event 하위만 허브가 허용)
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Categories = "AI.Event"))
	FGameplayTag EventTag = LSAITags::Event_Combat;

	// 이벤트 간격 최소 (초)
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float MinInterval = 3.0f;

	// 이벤트 간격 최대 (초)
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float MaxInterval = 6.0f;

	// 런타임 캐시: 소유 폰의 이벤트 허브
	UPROPERTY()
	TObjectPtr<ULSAIEventHubComponent> EventHub = nullptr;

	// 다음 이벤트까지 남은 시간
	float RemainingTime = 0.0f;
};

// 상태가 활성인 동안 Min~Max 랜덤 간격마다 AI 이벤트를 이벤트 허브로 보내는 Task.
// 항상 Running — 상태 전이로만 종료되므로 Global Task 또는 다른 Task와 병렬로 사용
USTRUCT(meta = (DisplayName = "LS Random Interval Event", Category = "LastStand|AI"))
struct FLSStateTreeRandomEventTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLSStateTreeRandomEventTaskInstanceData;

	FLSStateTreeRandomEventTask();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	// Min~Max 사이 다음 대기 시간 (Min > Max 설정 실수도 허용)
	static float PickInterval(const FInstanceDataType& InstanceData);
};
