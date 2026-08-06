// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSInventoryEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSInventoryEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 인벤토리 위젯이 호출해 1칸을 채움
	void SetItem(FName InItemID, int32 InQuantity, bool bInFromInventory, FGuid InInstanceID);
	
protected:
	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 우클릭 처리 — bFromInventory에 따라 인벤토리→장비 또는 월드후보→인벤토리로 분기
	void HandleRightClick();

	UPROPERTY(meta = (BindWidget)) 
	TObjectPtr<class UImage> IconImage;

	UPROPERTY(meta = (BindWidget)) 
	TObjectPtr<class UTextBlock> QuantityText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> NameText;

	FName ItemID; 

	int32 Quantity = 0; 

	FGuid InstanceID;

	bool bFromInventory = false;
};
