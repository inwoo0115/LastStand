// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void ActivateWidget();
	void DeActivateWidget();

	void SetLayerWidget(class ULSLayerWidget* Layer);

	void ToggleWidget();
	void ToggleWidget(bool ActiveState);

	virtual void ToggleWidgetByInput();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY()
	TObjectPtr<ULSLayerWidget> ParentLayer;

	bool bIsActive = false;
};
