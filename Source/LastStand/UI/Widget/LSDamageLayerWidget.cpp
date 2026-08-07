// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSDamageLayerWidget.h"
#include "UI/Widget/LSDamageNumberWidget.h"
#include "UI/LSUIEventSubsystem.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void ULSDamageLayerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Log, TEXT("ULSDamageLayerWidget Construct"));


	// 풀 사전 생성
	if (DamageWidgetClass && DamageCanvas)
	{
		for (int32 i = 0; i < PoolSize; ++i)
		{
			ULSDamageNumberWidget* W = CreateWidget<ULSDamageNumberWidget>(GetOwningPlayer(), DamageWidgetClass);
			if (!W)
			{
				continue;
			}

			UCanvasPanelSlot* CanvasSlot = DamageCanvas->AddChildToCanvas(W);
			if (CanvasSlot)
			{
				// 화면 중앙 앵커 — Position (0,0)이 정확히 중앙
				CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				CanvasSlot->SetAutoSize(true);
			}

			W->Deactivate();
			WidgetPool.Add(W);
		}
	}

	// DamageEvent 구독
	if (ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>())
	{
		DamageDelegateHandle = Sub->DamageEvent.AddUObject(this, &ULSDamageLayerWidget::HandleDamage);
	}
}

void ULSDamageLayerWidget::NativeDestruct()
{
	if (ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>())
	{
		if (DamageDelegateHandle.IsValid())
		{
			Sub->DamageEvent.Remove(DamageDelegateHandle);
		}
	}

	Super::NativeDestruct();
}

ULSDamageNumberWidget* ULSDamageLayerWidget::AcquireWidget()
{
	if (WidgetPool.Num() == 0)
	{
		return nullptr;
	}

	// 우선 비활성 위젯을 찾아 재사용
	for (const TObjectPtr<ULSDamageNumberWidget>& W : WidgetPool)
	{
		if (W && !W->IsBusy())
		{
			return W;
		}
	}

	// 전부 사용 중이면 라운드로빈으로 가장 오래된 것 재활용
	ULSDamageNumberWidget* Recycled = WidgetPool[NextIndex % WidgetPool.Num()];
	NextIndex = (NextIndex + 1) % WidgetPool.Num();
	return Recycled;
}

void ULSDamageLayerWidget::HandleDamage(int32 Damage)
{
	UE_LOG(LogTemp, Log, TEXT("Handle Damage UI"));

	ULSDamageNumberWidget* W = AcquireWidget();
	if (!W)
	{
		return;
	}

	// 화면 중앙 기준 랜덤 오프셋으로 밀집 배치
	const FVector2D Pos(
		FMath::FRandRange(-SpreadRange.X, SpreadRange.X),
		FMath::FRandRange(-SpreadRange.Y, SpreadRange.Y));

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(W->Slot))
	{
		CanvasSlot->SetPosition(Pos);
	}

	W->ShowDamage(Damage);
}
