// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LSWidgetBase.h"
#include "LSInventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSInventoryWidget : public ULSWidgetBase
{
	GENERATED_BODY()
	
public:
    virtual void ToggleWidgetByInput() override;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    FDelegateHandle InputDelegate;

    UPROPERTY(meta = (BindWidget)) 
    TObjectPtr<class ULSInventorySlotWidget> InventoryContainer;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class ULSInteractionSlotWidget> DropItemContainer;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class ULSEquipmentSlotWidget> EquipmentContainer;
};
