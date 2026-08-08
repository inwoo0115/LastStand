// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LSWidgetBase.h"
#include "LSPlayerStatWidget.generated.h"

/**
 * 플레이어 본인의 현재/최대 체력을 표시하는 HUD 위젯.
 * ULSUIEventSubsystem::HealthEvent(현재, 최대)를 구독해 갱신한다. (클라 HUD 전용)
 */
UCLASS()
class LASTSTAND_API ULSPlayerStatWidget : public ULSWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// HealthEvent 수신 → 텍스트 갱신
	void HandleHealthEvent(int32 Current, int32 Max);

	void UpdateHealthText(int32 Current, int32 Max);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> CurrentHealthText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> MaxHealthText;

	FDelegateHandle HealthDelegateHandle;
};
