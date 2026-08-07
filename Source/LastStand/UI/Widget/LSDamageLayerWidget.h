// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSDamageLayerWidget.generated.h"

/**
 * ULSDamageNumberWidget을 오브젝트 풀로 관리하는 레이어 위젯.
 * 생성 시 PoolSize만큼 미리 생성하고, DamageEvent를 구독해 방송이 오면
 * 풀에서 하나를 꺼내 화면 중앙 부근 랜덤 위치에 표시한다.
 */
UCLASS()
class LASTSTAND_API ULSDamageLayerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// DamageEvent 수신 핸들러
	void HandleDamage(int32 Damage);

	// 풀에서 사용 가능한(또는 재활용) 위젯 획득
	class ULSDamageNumberWidget* AcquireWidget();

	// 데미지 위젯이 배치될 루트 캔버스 (BP에서 이름 DamageCanvas)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> DamageCanvas;

	// 풀에 생성할 데미지 위젯 클래스 (WBP_DamageNumber)
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<class ULSDamageNumberWidget> DamageWidgetClass;

	// 미리 생성할 풀 크기
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	int32 PoolSize = 12;

	// 중앙 기준 랜덤 오프셋 반경 (X, Y) — 중앙 밀집도 조절
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FVector2D SpreadRange = FVector2D(160.f, 120.f);

	UPROPERTY()
	TArray<TObjectPtr<ULSDamageNumberWidget>> WidgetPool;

	int32 NextIndex = 0;
	FDelegateHandle DamageDelegateHandle;
};
