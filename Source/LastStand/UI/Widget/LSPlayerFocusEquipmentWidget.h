// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LSWidgetBase.h"
#include "LSPlayerFocusEquipmentWidget.generated.h"

/**
 * 현재 포커스(활성) 무기의 아이콘 + 현재/최대 탄약을 표시하는 HUD 위젯.
 * 무기 활성화 시 UI.Slot.Weapon 슬롯에 주입되며, ULSUIEventSubsystem::AmmoEvent로 탄약을 갱신한다. (클라 HUD 전용)
 */
UCLASS()
class LASTSTAND_API ULSPlayerFocusEquipmentWidget : public ULSWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 포커스 무기 아이콘 pull
	void RefreshIcon();

	// AmmoEvent 수신 → 탄약 텍스트 갱신
	void HandleAmmoChanged(int32 Current, int32 Max);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> WeaponIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> CurrentAmmoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> MaxAmmoText;

	FDelegateHandle AmmoHandle;
	FDelegateHandle FocusHandle;
};
