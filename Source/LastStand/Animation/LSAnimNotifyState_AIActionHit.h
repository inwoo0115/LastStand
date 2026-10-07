// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "LSAnimNotifyState_AIActionHit.generated.h"

// AI 행동 몽타주의 히트 판정 구간. 서버에서만 AI 액션 컴포넌트의 현재 행동에 전달
UCLASS(meta = (DisplayName = "LS AI Action Hit Window"))
class LASTSTAND_API ULSAnimNotifyState_AIActionHit : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	// 행동이 여러 구간을 구분할 때 사용하는 이름
	UPROPERTY(EditAnywhere, Category = "AI Action")
	FName WindowName = NAME_None;

	void SendHitWindow(USkeletalMeshComponent* MeshComp, bool bBegin) const;
};
