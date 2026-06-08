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
	void SetItem(FName InItemID, int32 InQuantity, bool bInFromInventory);
	
protected:
	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget)) 
	TObjectPtr<class UImage> IconImage;

	UPROPERTY(meta = (BindWidget)) 
	TObjectPtr<class UTextBlock> QuantityText;

	FName ItemID; 

	int32 Quantity = 0; 

	bool bFromInventory = false;
};
