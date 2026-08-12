// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSEnemyStatWidget.h"
#include "Character/Components/LSStatComponent.h"
#include "Components/ProgressBar.h"

void ULSEnemyStatWidget::InitializeWidget(ULSStatComponent* InStatComp)
{
	if (!InStatComp)
	{
		return;
	}

	StatComp = InStatComp;

	// 초기 체력바 세팅
	UpdateHealthBar(StatComp->GetCurrentHealth(), StatComp->GetMaxHealth());

	// 체력 변경 구독 (클라는 OnRep_CurrentHealth가 브로드캐스트)
	StatComp->OnHealthChanged.AddUObject(this, &ULSEnemyStatWidget::HandleHealthChanged);
}

void ULSEnemyStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 피해를 입기 전엔 숨겨둔다 (InitializeWidget/UpdateHealthBar에서 피격 시 표시)
	SetVisibility(ESlateVisibility::Collapsed);
}

void ULSEnemyStatWidget::NativeDestruct()
{
	if (StatComp)
	{
		StatComp->OnHealthChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void ULSEnemyStatWidget::HandleHealthChanged(int32 NewCurrent, int32 NewMax)
{
	UpdateHealthBar(NewCurrent, NewMax);
}

void ULSEnemyStatWidget::UpdateHealthBar(int32 Current, int32 Max)
{
	if (!HealthBar)
	{
		return;
	}

	const float Percent = (Max > 0) ? static_cast<float>(Current) / static_cast<float>(Max) : 0.0f;
	HealthBar->SetPercent(Percent);

	// 피해를 입기 전(풀피)엔 숨기고, 피해를 입은 뒤부터 표시 (비상호작용 월드 HUD)
	const bool bDamaged = Current < Max;
	SetVisibility(bDamaged ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
