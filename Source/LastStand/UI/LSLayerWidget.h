// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "LSUISubsystem.h"
#include "LSLayerWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSLayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UUserWidget* PushWidget(TSubclassOf<UUserWidget> WidgetClass);
	void PopWidget(UUserWidget* Widget);

	bool ContainsWidget(UUserWidget* Widget) const;

	void ActivateLayer();

	void DeactivateLayer();

	bool GetIsActivated();

	EInputType GetInputType();
	
protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	bool bIsActivated = false;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOverlay> WidgetContainer;

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> WidgetStack;

	UPROPERTY(EditAnywhere, Category = "Input Type")
	EInputType InputType = EInputType::Game;
	
};
