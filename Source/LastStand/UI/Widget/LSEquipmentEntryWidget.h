// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSEquipmentEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSEquipmentEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetEntry(FName InItemID);


protected:
	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	
	
};
