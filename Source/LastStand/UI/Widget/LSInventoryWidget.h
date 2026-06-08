// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSInventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    UPROPERTY(meta = (BindWidget)) 
    TObjectPtr<class ULSInventorySlotWidget> InventoryContainer;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class ULSInteractionSlotWidget> DropItemContainer;
};
