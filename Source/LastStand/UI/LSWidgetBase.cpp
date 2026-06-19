// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LSWidgetBase.h"
#include "LSWidgetBase.h"
#include "LSLayerWidget.h"

void ULSWidgetBase::DeActivateWidget()
{
	ParentLayer->PopWidgetFromWidgetStack(this);

	ParentLayer->DeactivateLayerIfEmpty();

	ToggleWidget(false);
}

void ULSWidgetBase::ActivateWidget()
{
	ParentLayer->PushWidgetToWidgetStack(this);

	ParentLayer->ActivateLayer();

	ToggleWidget(true);
}

void ULSWidgetBase::SetLayerWidget(ULSLayerWidget* Layer)
{
	ParentLayer = Layer;
}

void ULSWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULSWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void ULSWidgetBase::ToggleWidget()
{
	if (!bIsActive)
	{
		bIsActive = true;
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		bIsActive = false;
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ULSWidgetBase::ToggleWidget(bool ActiveState)
{
	if (bIsActive == ActiveState)
	{
		return;
	}

	ToggleWidget();
}

void ULSWidgetBase::ToggleWidgetByInput()
{
}

