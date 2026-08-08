// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSDamageNumberWidget.generated.h"

class UWidgetAnimation;

/**
 * 데미지 숫자 하나를 표시하는 단일 위젯. LSDamageLayerWidget이 오브젝트 풀로 관리한다.
 */
UCLASS()
class LASTSTAND_API ULSDamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 데미지 값을 표시하고 DisplayDuration 후 자동 해제
	void ShowDamage(int32 Damage);

	// 즉시 해제 (풀에서 비활성 상태로 전환)
	void Deactivate();

	// 풀에서 사용 중인지 여부 (비활성=Collapsed)
	bool IsBusy() const { return GetVisibility() != ESlateVisibility::Collapsed; }

protected:
	virtual void NativeConstruct() override;

	// 애니메이션 종료 시 자동 해제
	UFUNCTION()
	void OnShowAnimFinished();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> DamageText;

	// WBP의 애니메이션과 이름이 정확히 일치해야 함 (ShowAnim). 없으면 폴백 타이머 사용
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ShowAnim;

	// 애니메이션 미바인딩 시 폴백 표시 지속 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DisplayDuration = 1.0f;

	FTimerHandle HideTimerHandle;
};
