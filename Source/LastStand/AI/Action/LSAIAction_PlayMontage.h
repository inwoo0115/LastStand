// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Action/LSAIActionBase.h"
#include "LSAIAction_PlayMontage.generated.h"

// 범용 연출 액션: 몽타주 재생 → 정상 종료 시 EventOnFinished를 StateTree에 전송 (등장/포효/패턴 전환 등)
// 상태 이탈(Cancel)·다른 몽타주에 끊기면(피격·사망) 이벤트 미전송
UCLASS(meta = (DisplayName = "Play Montage"))
class LASTSTAND_API ULSAIAction_PlayMontage : public ULSAIActionBase
{
	GENERATED_BODY()

public:
	ULSAIAction_PlayMontage();

protected:
	virtual void OnEnd(bool bSucceeded) override;

	// 몽타주 정상 종료 시 전송할 이벤트 (비어 있으면 전송 안 함)
	UPROPERTY(EditAnywhere, Category = "Event", meta = (Categories = "AI.Event"))
	FGameplayTag EventOnFinished;
};
