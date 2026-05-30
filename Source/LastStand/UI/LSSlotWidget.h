// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "LSSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    void AddWidget(TSubclassOf<UUserWidget> WidgetClass, int32 HandleId);
    void RemoveWidget(int32 HandleId);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Slot")
    FGameplayTag SlotTag;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UVerticalBox> Container;

    UPROPERTY()
    TMap<int32, TObjectPtr<UUserWidget>> InjectedWidgets;
};
