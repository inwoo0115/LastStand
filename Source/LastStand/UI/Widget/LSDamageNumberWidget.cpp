// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSDamageNumberWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "TimerManager.h"
#include "Engine/World.h"

void ULSDamageNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 풀 초기 상태: 비활성
	SetVisibility(ESlateVisibility::Collapsed);

	// 애니메이션 종료 시 자동 해제 델리게이트 바인딩
	if (ShowAnim)
	{
		FWidgetAnimationDynamicEvent Finished;
		Finished.BindDynamic(this, &ULSDamageNumberWidget::OnShowAnimFinished);
		BindToAnimationFinished(ShowAnim, Finished);
	}
}

void ULSDamageNumberWidget::ShowDamage(int32 Damage)
{
	if (DamageText)
	{
		DamageText->SetText(FText::AsNumber(Damage));
	}

	// HUD 텍스트는 입력을 가로채지 않도록 HitTestInvisible
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (ShowAnim)
	{
		// 애니메이션이 수명 전체 주도 (재활용 시에도 처음부터 재시작)
		PlayAnimation(ShowAnim);
	}
	else if (UWorld* World = GetWorld())
	{
		// 폴백: 애니메이션 미바인딩 시 타이머로 해제
		World->GetTimerManager().SetTimer(HideTimerHandle, this, &ULSDamageNumberWidget::Deactivate, DisplayDuration, false);
	}
}

void ULSDamageNumberWidget::OnShowAnimFinished()
{
	Deactivate();
}

void ULSDamageNumberWidget::Deactivate()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}
