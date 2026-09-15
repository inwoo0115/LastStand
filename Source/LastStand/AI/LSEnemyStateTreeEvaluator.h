// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeEvaluatorBase.h"
#include "LSEnemyStateTreeEvaluator.generated.h"

struct FStateTreeExecutionContext;
class ULSAIPerceptionComponent;

// Evaluator가 StateTree에 노출/보관할 인스턴스 데이터.
USTRUCT()
struct FLSEnemyStateTreeEvaluatorInstanceData
{
	GENERATED_BODY()

	// 출력: 퍼셉션이 선정한 가장 가까운 타깃 (StateTree 바인딩 소스)
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> TargetActor = nullptr;

	// 런타임 캐시: 소유 폰의 퍼셉션 컴포넌트 (TreeStart에서 해석)
	UPROPERTY()
	TObjectPtr<ULSAIPerceptionComponent> Perception = nullptr;
};

// 적 AI 결정용 데이터를 계산·노출하는 Evaluator (현재는 골격만).
USTRUCT(meta = (DisplayName = "LS Enemy Evaluator", Category = "LastStand|AI"))
struct FLSEnemyStateTreeEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLSEnemyStateTreeEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

protected:
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
