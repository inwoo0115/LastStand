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

	const float ViewYaw = FRotator::ClampAxis(PC->GetControlRotation().Yaw + NorthYawOffset);

	if (CompassStrip && StripCycles > 0)
	{
		// 360°의 픽셀 폭을 스트립 실제 렌더 폭에서 산출(손 보정 불필요). 첫 틱엔 0일 수 있어 가드.
		const float StripW = CompassStrip->GetCachedGeometry().GetLocalSize().X;
		if (StripW > 0.0f)
		{
			const float PixelsPer360 = StripW / StripCycles;
			const float PixelsPerDeg = PixelsPer360 / 360.0f;
			const float WindowW = MyGeometry.GetLocalSize().X;

			// 스트립 왼쪽 끝(StripLeftHeading) 기준 현재 heading까지의 각도 → 스트립 내 픽셀 위치
			const float DegFromLeft = FRotator::ClampAxis(ViewYaw - StripLeftHeading); // 0~360
			const float HeadingX = DegFromLeft * PixelsPerDeg;

			// 스트립은 360° 주기로 반복(2연속/타일)이므로 Translate를 한 주기(-PixelsPer360, 0] 안으로
			// 정규화해 좌/우 회전 모두 창이 항상 채워지게 함
			float Translate = FMath::Fmod(WindowW * 0.5f - HeadingX, PixelsPer360);
			if (Translate > 0.0f)
			{
				Translate -= PixelsPer360;
			}

			CompassStrip->SetRenderTranslation(FVector2D(Translate, 0.0f));
		}
	}

	// 현재 방위+각도 텍스트 갱신
	if (HeadingText)
	{
		const FString HeadingStr = FString::Printf(TEXT("%s %d°"), *HeadingToCardinal(ViewYaw), FMath::RoundToInt(ViewYaw));
		HeadingText->SetText(FText::FromString(HeadingStr));
	}
}

FString ULSCompassBarWidget::HeadingToCardinal(float Yaw)
{
	static const TCHAR* Cardinals[] = { TEXT("N"), TEXT("NE"), TEXT("E"), TEXT("SE"), TEXT("S"), TEXT("SW"), TEXT("W"), TEXT("NW") };

	const int32 Index = FMath::RoundToInt(Yaw / 45.0f) % 8;
	return Cardinals[Index];
}
