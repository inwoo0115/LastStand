// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LSLayerWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "LSUISubsystem.h"

UUserWidget* ULSLayerWidget::PushWidget(TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass || !WidgetContainer) return nullptr;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return nullptr;

    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (!NewWidget) return nullptr;

    UOverlaySlot* OLSlot = WidgetContainer->AddChildToOverlay(NewWidget);
    ensureMsgf(OLSlot, TEXT("LSLayerWidget: AddChildToOverlay returned null slot"));
    if (OLSlot)
    {
        OLSlot->SetHorizontalAlignment(HAlign_Fill);
        OLSlot->SetVerticalAlignment(VAlign_Fill);
    }

    WidgetStack.Add(NewWidget);
    return NewWidget;
}

void ULSLayerWidget::PopWidget(UUserWidget* Widget)
{
    if (!Widget || !WidgetContainer) return;
    WidgetContainer->RemoveChild(Widget);
    WidgetStack.Remove(Widget);
}

bool ULSLayerWidget::ContainsWidget(UUserWidget* Widget) const
{
    return WidgetStack.Contains(Widget);

}

void ULSLayerWidget::ActivateLayer()
{
    SetVisibility(ESlateVisibility::Visible);

    bIsActivated = true;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (ULSUISubsystem* Subsystem = GI->GetSubsystem<ULSUISubsystem>())
        {
            Subsystem->UpdateInputType();
        }
    }
}

void ULSLayerWidget::DeactivateLayer()
{
    SetVisibility(ESlateVisibility::Collapsed);

    bIsActivated = false;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (ULSUISubsystem* Subsystem = GI->GetSubsystem<ULSUISubsystem>())
        {
            Subsystem->UpdateInputType();
        }
    }
}

bool ULSLayerWidget::GetIsActivated()
{
    return bIsActivated;
}

EInputType ULSLayerWidget::GetInputType()
{
    return InputType;
}

void ULSLayerWidget::NativeConstruct()
{
    Super::NativeConstruct();
}
