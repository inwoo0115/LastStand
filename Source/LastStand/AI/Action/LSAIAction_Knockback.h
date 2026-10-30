// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Action/LSAIActionBase.h"
#include "LSAIAction_Knockback.generated.h"

// 주변 넉백: 히트 구간 시작 시점에 소유자 반경 내 캐릭터를 바깥쪽으로 밀어냄 (구간 종료는 무시)
UCLASS(meta = (DisplayName = "Knockback"))
class LASTSTAND_API ULSAIAction_Knockback : public ULSAIActionBase
{
	GENERATED_BODY()

public:
	virtual void OnHitWindow(FName WindowName, bool bBegin) override;

protected:
	// 반경 내 대상에 바깥 방향 힘 적용 (서버)
	void ApplyKnockback();

	// 넉백 반경
	UPROPERTY(EditAnywhere, Category = "Knockback", meta = (ClampMin = "0.0"))
	float KnockbackRadius = 500.0f;

	// 수평(바깥쪽) 힘
	UPROPERTY(EditAnywhere, Category = "Knockback", meta = (ClampMin = "0.0"))
	float KnockbackStrength = 1200.0f;

	// 수직 힘 (살짝 띄워야 지면 마찰에 덜 막힘)
	UPROPERTY(EditAnywhere, Category = "Knockback", meta = (ClampMin = "0.0"))
	float KnockbackUpStrength = 300.0f;

	// 특정 구간만 반응 (None이면 모든 구간 시작에 반응)
	UPROPERTY(EditAnywhere, Category = "Knockback")
	FName KnockbackWindowName = NAME_None;
};
