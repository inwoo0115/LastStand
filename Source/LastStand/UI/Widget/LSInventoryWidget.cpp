// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSInventoryWidget.h"
#include "UI/LSUIEventSubsystem.h"

void ULSInventoryWidget::ToggleWidgetByInput()
{
	if (bIsActive)
	{
		DeActivateWidget();
	}
	else
	{
		ActivateWidget();
	}
}

void ULSInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>();
	if (Sub)
	{
		InputDelegate = Sub->InventoryInput.AddUObject(this, &ULSInventoryWidget::ToggleWidgetByInput);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void ULSInventoryWidget::NativeDestruct()
{
	Super::NativeDestruct();

	ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>();
	if (Sub && InputDelegate.IsValid())
	{
		Sub->InventoryInput.Remove(InputDelegate);
	}
}
