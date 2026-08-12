// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LSWidgetBase.h"
#include "LSCompassBarWidget.generated.h"

/**
 * 플레이어가 바라보는 방향을 표시하는 나침반 HUD 위젯.
 * 시야 yaw(GetControlRotation().Yaw)에 따라 눈금 스트립을 스크롤시키고,
 * 현재 방위(8방위: N/NE/E/SE/S/SW/W/NW)와 각도를 텍스트로 표시한다. (클라 HUD 전용)
 *
 * 위젯 루트를 ClipToBounds 고정 크기 클립보드로 삼아 그 폭만 노출하고,
 * CompassStrip을 RenderTranslation으로 좌우 이동시켜 스크롤되게 한다.
 * 스트립은 자기 크기를 유지하며(부모 크기에 종속되지 않음) 창을 넘치는 부분만 잘린다.
 */
UCLASS()
class LASTSTAND_API ULSCompassBarWidget : public ULSWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// yaw(0~360)를 8방위 문자열로 변환
	static FString HeadingToCardinal(float Yaw);

	// 스크롤되는 눈금/방위 스트립. RenderTranslation.X로 이동시킨다.
	// 라벨은 0°~720°(두 주기) 반복으로 그려 랩어라운드가 끊기지 않게 한다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidget> CompassStrip;

	// 현재 방위+각도 텍스트 (예: "NE 47°"). 배치하지 않아도 동작.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> HeadingText;

	// 스트립 위젯에 그려 넣은 360° 사이클 수 (끊김 없는 랩을 위해 보통 2 = 720°).
	// 360°의 픽셀 폭(PixelsPer360)은 스트립 실제 폭 / StripCycles로 매 틱 자동 산출한다.
	UPROPERTY(EditDefaultsOnly, Category = "Compass")
	int32 StripCycles = 2;

	// 스트립 이미지의 왼쪽 끝이 나타내는 heading(도). 제공 이미지는 왼쪽 끝이 W=270°.
	UPROPERTY(EditDefaultsOnly, Category = "Compass")
	float StripLeftHeading = 270.0f;

	// 월드 yaw → 방위 보정. 레벨에서 '북쪽'으로 삼을 축에 맞춘다.
	UPROPERTY(EditDefaultsOnly, Category = "Compass")
	float NorthYawOffset = 0.0f;
};
