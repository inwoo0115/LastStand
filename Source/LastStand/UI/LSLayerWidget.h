// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
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

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOverlay> WidgetContainer;

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> WidgetStack;
	
};
