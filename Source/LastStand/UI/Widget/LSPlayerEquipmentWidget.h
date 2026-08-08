// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LSWidgetBase.h"
#include "DataTable/LSWeaponData.h"   // EEquipmentType
#include "LSPlayerEquipmentWidget.generated.h"

/**
 * 플레이어 장착 장비(Main/Sub/Throwable/Melee) 4칸을 아이콘+이름으로 표시하는 HUD 위젯.
 * ULSUIEventSubsystem의 EquipmentArrayChanged/FocusEquipmentChanged를 구독하고,
 * 캐싱한 ULSEquipmentComponent를 이벤트 시에만 읽어 갱신한다. (클라 HUD 전용)
 */
UCLASS()
class LASTSTAND_API ULSPlayerEquipmentWidget : public ULSWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 장비 배열 변경 → 4칸 아이콘/이름 재갱신 + 포커스 opacity
	void HandleArrayChanged();

	// 포커스 변경 → 포커스 opacity만 갱신
	void HandleFocusChanged();

	// 캐시된 장비 컴포넌트 확보(무효 시 로컬 폰에서 재취득)
	class ULSEquipmentComponent* EnsureEquipmentComp();

	// 한 슬롯 갱신 (장착 무기 있으면 아이콘/이름, 없으면 비움)
	void UpdateSlot(EEquipmentType Type, class UImage* Icon, class UTextBlock* NameText);

	// 포커스=1.0, 나머지=UnfocusedOpacity로 각 슬롯 아이콘/이름 RenderOpacity 반영
	void RefreshFocusOpacity();

	// 한 슬롯의 아이콘/이름 opacity 적용 (포커스면 1.0, 아니면 UnfocusedOpacity)
	void ApplySlotOpacity(class UImage* Icon, class UTextBlock* NameText, bool bFocused);

	// --- Main ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> MainIcon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> MainNameText;

	// --- Sub ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> SubIcon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> SubNameText;

	// --- Throwable ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ThrowableIcon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ThrowableNameText;

	// --- Melee ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> MeleeIcon;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> MeleeNameText;

	// 비포커스 슬롯 디밍 정도(0~1)
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	float UnfocusedOpacity = 0.4f;

	UPROPERTY()
	TWeakObjectPtr<class ULSEquipmentComponent> EquipmentComp;

	FDelegateHandle ArrayChangedHandle;
	FDelegateHandle FocusChangedHandle;
};
