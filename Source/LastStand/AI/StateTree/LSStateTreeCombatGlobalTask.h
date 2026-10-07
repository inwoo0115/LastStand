// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeTaskBase.h"
#include "LSStateTreeCombatGlobalTask.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeTransitionResult;
class ULSAIPerceptionComponent;
class ALSEnemyBase;

// Combat Global Task가 StateTree에 노출/보관할 인스턴스 데이터.
USTRUCT()
struct FLSStateTreeCombatGlobalTaskInstanceData
{
	GENERATED_BODY()

	// 출력: 퍼셉션이 선정한 가장 가까운 타깃 (StateTree 바인딩 소스)
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> TargetActor = nullptr;

	// 출력: 소유 적의 현재 페이즈 (0 = 시작, 체력 임계치 발동 시 갱신) — 조건 바인딩 소스
	UPROPERTY(EditAnywhere, Category = "Output")
	int32 CurrentPhase = 0;

	// 런타임 캐시: 소유 폰의 퍼셉션 컴포넌트 (EnterState에서 해석)
	UPROPERTY()
	TObjectPtr<ULSAIPerceptionComponent> Perception = nullptr;

	// 런타임 캐시: 소유 폰 (EnterState에서 해석)
	UPROPERTY()
	TObjectPtr<ALSEnemyBase> Enemy = nullptr;
};

// Root에서 타겟 선정·엄폐 후보 선정 등을 담당하는 전투 Global Task.
USTRUCT(meta = (DisplayName = "LS Combat Global Task", Category = "LastStand|AI"))
struct FLSStateTreeCombatGlobalTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLSStateTreeCombatGlobalTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	// 캐시로부터 Output(TargetActor, CurrentPhase) 갱신 — EnterState/Tick 공용
	static void UpdateOutputs(FInstanceDataType& InstanceData);
};
