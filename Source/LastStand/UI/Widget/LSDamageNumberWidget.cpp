// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSDamageNumberWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Engine/World.h"

void ULSDamageNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 풀 초기 상태: 비활성
	SetVisibility(ESlateVisibility::Collapsed);
}

void ULSDamageNumberWidget::ShowDamage(int32 Damage)
{
	if (DamageText)
	{
		DamageText->SetText(FText::AsNumber(Damage));
	}

	// HUD 텍스트는 입력을 가로채지 않도록 HitTestInvisible
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// 표시 시간 후 자동 해제 (재활용 시 타이머 리셋)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HideTimerHandle, this, &ULSDamageNumberWidget::Deactivate, DisplayDuration, false);
	}
}

void ULSDamageNumberWidget::Deactivate()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}
