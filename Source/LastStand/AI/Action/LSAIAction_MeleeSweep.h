// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Action/LSAIActionBase.h"
#include "LSAIAction_MeleeSweep.generated.h"

// 근접 공격: 히트 구간 동안 메시 소켓 궤적을 구체 스윕해 데미지 (액션당 대상별 1회)
UCLASS(meta = (DisplayName = "Melee Sweep"))
class LASTSTAND_API ULSAIAction_MeleeSweep : public ULSAIActionBase
{
	GENERATED_BODY()

public:
	virtual void TickAction(float DeltaTime) override;

	virtual void OnHitWindow(FName WindowName, bool bBegin) override;

protected:
	virtual bool OnActivate() override;

	virtual void OnEnd(bool bSucceeded) override;

	// 직전 위치 → 현재 소켓 위치로 스윕 후 데미지 적용
	void SweepAndApplyDamage();

	bool GetTraceLocation(FVector& OutLocation) const;

	// 스윕 기준 메시 소켓 (무기 끝/손 등)
	UPROPERTY(EditAnywhere, Category = "Melee")
	FName TraceSocket = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Melee", meta = (ClampMin = "1.0"))
	float TraceRadius = 30.0f;

	bool bHitWindowOpen = false;

	FVector LastTraceLocation = FVector::ZeroVector;

	// 이번 실행에서 이미 맞은 대상 (중복 히트 방지)
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
