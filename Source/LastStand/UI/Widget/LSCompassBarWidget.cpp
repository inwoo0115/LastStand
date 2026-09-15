// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSCompassBarWidget.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"

void ULSCompassBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	float ViewYaw = FRotator::ClampAxis(PC->GetControlRotation().Yaw + NorthYawOffset);
	if (ViewYaw >= 180.0f)
	{
		ViewYaw -= 360.0f;
	}


	if (CompassStrip && StripCycles > 0)
	{
		// 첫 틱엔 0일 수 있음 기본값 2048
		const float StripW = CompassStrip->GetCachedGeometry().GetLocalSize().X;
		if (StripW > 0.0f)
		{
			const float PixelsPer360 = StripW / StripCycles; //1024
			const float PixelsPerDeg = PixelsPer360 / 360.0f;

			float HeadingX = ViewYaw * PixelsPerDeg * -1.0f;

			CompassStrip->SetRenderTranslation(FVector2D(HeadingX, 0.0f));
		}
	}

	// 현재 방위+각도 텍스트 갱신
	if (HeadingText)
	{
		const FString HeadingStr = FString::Printf(TEXT("%s %d°"), *HeadingToCardinal(FRotator::ClampAxis(PC->GetControlRotation().Yaw)), FMath::RoundToInt(ViewYaw));
		HeadingText->SetText(FText::FromString(HeadingStr));
	}
}

FString ULSCompassBarWidget::HeadingToCardinal(float Yaw)
{
	static const TCHAR* Cardinals[] = { TEXT("N"), TEXT("NE"), TEXT("E"), TEXT("SE"), TEXT("S"), TEXT("SW"), TEXT("W"), TEXT("NW") };

	const int32 Index = FMath::RoundToInt(Yaw / 45.0f) % 8;
	return Cardinals[Index];
}
