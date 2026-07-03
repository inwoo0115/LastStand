// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSEquipmentSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void RefreshSlot();
	

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual bool NativeOnDragOver(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;
};
