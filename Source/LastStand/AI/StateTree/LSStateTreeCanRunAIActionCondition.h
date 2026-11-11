// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeConditionBase.h"
#include "GameplayTagContainer.h"
#include "LSStateTreeCanRunAIActionCondition.generated.h"

struct FStateTreeExecutionContext;

USTRUCT()
struct FLSStateTreeCanRunAIActionConditionInstanceData
{
	GENERATED_BODY()

	// 검사할 행동 (EnemyData의 Actions 키)
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Categories = "AI.Action"))
	FGameplayTag ActionTag;

	// 대상 (Combat Global Task의 TargetActor 바인딩) — 사거리 검사 기준
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<AActor> TargetActor = nullptr;

	// 실행 중인 액션을 무시하고 대상 액션의 실행 가능 여부(쿨다운·사거리)만 검사
	// (전이 시 현재 액션이 이탈 상태의 ExitState에서 취소되는 경우에 사용)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bIgnoreCurrentAction = false;
};

// 행동 실행 가능 여부(실행 중 아님 + 쿨다운 + 사거리). 공격 상태의 Enter Condition으로 사용
USTRUCT(meta = (DisplayName = "LS Can Run AI Action", Category = "LastStand|AI"))
struct FLSStateTreeCanRunAIActionCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLSStateTreeCanRunAIActionConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};
