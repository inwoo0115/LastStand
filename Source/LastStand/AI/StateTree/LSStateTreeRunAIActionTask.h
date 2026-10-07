// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "LSStateTreeRunAIActionTask.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;
class ULSAIActionComponent;

USTRUCT()
struct FLSStateTreeRunAIActionTaskInstanceData
{
	GENERATED_BODY()

	// 실행할 행동 (EnemyData의 Actions 키)
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Categories = "AI.Action"))
	FGameplayTag ActionTag;

	// 대상 (Combat Global Task의 TargetActor 바인딩)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<AActor> TargetActor = nullptr;

	// 런타임 캐시: 소유 폰의 행동 컴포넌트
	UPROPERTY()
	TObjectPtr<ULSAIActionComponent> ActionComp = nullptr;

	// 행동 종료 델리게이트 구독 핸들
	FDelegateHandle FinishedHandle;
};

// AI 액션 컴포넌트에 태그로 행동을 실행시키는 범용 Task.
// 행동 종료 시 성공/실패로 Task 완료, 상태 이탈(피격·사망 전이 등) 시 행동 중단
USTRUCT(meta = (DisplayName = "LS Run AI Action", Category = "LastStand|AI"))
struct FLSStateTreeRunAIActionTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLSStateTreeRunAIActionTaskInstanceData;

	FLSStateTreeRunAIActionTask();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
