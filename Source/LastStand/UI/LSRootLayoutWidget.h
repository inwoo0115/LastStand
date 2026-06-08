// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"

#include "LSRootLayoutWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSRootLayoutWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class ULSLayerWidget> LayerGame;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<ULSLayerWidget> LayerMenu;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<ULSLayerWidget> LayerModal;
	
};
