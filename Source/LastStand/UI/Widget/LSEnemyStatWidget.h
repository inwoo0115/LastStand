// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSEnemyStatWidget.generated.h"

/**
 *
 */
UCLASS()
class LASTSTAND_API ULSEnemyStatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 소유 pawn의 stat 컴포넌트로 초기화 + 체력변경 구독 (클라에서 호출)
	void InitializeWidget(class ULSStatComponent* InStatComp);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// OnHealthChanged 델리게이트 콜백
	void HandleHealthChanged(int32 NewCurrent, int32 NewMax);

	void UpdateHealthBar(int32 Current, int32 Max);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> HealthBar;

	UPROPERTY()
	TObjectPtr<class ULSStatComponent> StatComp;
};
