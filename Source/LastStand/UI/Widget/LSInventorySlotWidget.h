// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSInventorySlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void RefreshSlot();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 드랍 받기: 월드 후보를 인벤토리로 → 줍기
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual bool NativeOnDragOver(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> Container;

	UPROPERTY()
	TWeakObjectPtr<class ULSInventoryComponent> InventoryComp;

	UPROPERTY()
	TWeakObjectPtr<class ULSInteractionComponent> InteractionComp;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<class ULSInventoryEntryWidget> EntryWidgetClass;  // 1칸 위젯 클래스
};
