// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LSLayerWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "LSUISubsystem.h"
#include "LSWidgetBase.h"

UUserWidget* ULSLayerWidget::PushWidget(TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass || !WidgetContainer) return nullptr;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return nullptr;

    ULSWidgetBase* NewWidget = CreateWidget<ULSWidgetBase>(PC, WidgetClass);
    if (!NewWidget) return nullptr;

    UOverlaySlot* OLSlot = WidgetContainer->AddChildToOverlay(NewWidget);
    ensureMsgf(OLSlot, TEXT("LSLayerWidget: AddChildToOverlay returned null slot"));
    if (OLSlot)
    {
        OLSlot->SetHorizontalAlignment(HAlign_Fill);
        OLSlot->SetVerticalAlignment(VAlign_Fill);
    }

    NewWidget->SetLayerWidget(this);

    return NewWidget;
}

void ULSLayerWidget::PopWidget(UUserWidget* Widget)
{
    if (!Widget || !WidgetContainer)
    {
        return;
    }
    WidgetContainer->RemoveChild(Widget);

    // 열린 채로 제거되는 위젯이 스택에 남지 않도록
    WidgetStack.RemoveSingle(Widget);
}

bool ULSLayerWidget::ContainsWidget(UUserWidget* Widget) const
{
    // 열림 여부와 무관하게 이 레이어 Overlay에 붙어 있는 위젯인지
    return WidgetContainer && WidgetContainer->HasChild(Widget);
}

void ULSLayerWidget::ActivateLayer()
{
    if (bIsActivated)
    {
        return;
    }

    SetVisibility(ESlateVisibility::Visible);

    bIsActivated = true;

    if (CachedUISubsystem)
    {
        CachedUISubsystem->UpdateInputType();
    }
}

void ULSLayerWidget::DeactivateLayer()
{
    if (!bIsActivated)
    {
        return;
    }

    SetVisibility(ESlateVisibility::Collapsed);

    bIsActivated = false;

    if (CachedUISubsystem)
    {
        CachedUISubsystem->UpdateInputType();
    }
}

void ULSLayerWidget::DeactivateLayerIfEmpty()
{
    if (WidgetStack.IsEmpty())
    {
        DeactivateLayer();
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

void ULSLayerWidget::PushWidgetToWidgetStack(UUserWidget* Widget)
{
    if (!Widget)
    {
        return;
    }

    // 이미 있으면 지우고 최상단으로
    WidgetStack.RemoveSingle(Widget);

    WidgetStack.Push(Widget);

    //Zorder 맨위로
    UOverlaySlot* OLSlot = WidgetContainer->AddChildToOverlay(Widget);
    if (OLSlot)
    {
        OLSlot->SetHorizontalAlignment(HAlign_Fill);
        OLSlot->SetVerticalAlignment(VAlign_Fill);
    }
}

void ULSLayerWidget::PopWidgetFromWidgetStack(UUserWidget* Widget)
{
    if (!Widget)
    {
        return;
    }

    WidgetStack.RemoveSingle(Widget);
}

void ULSLayerWidget::FocusOnTopWidget()
{
    if (!WidgetStack.IsEmpty())
    {
        WidgetStack.Top()->SetFocus();
    }
}

void ULSLayerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (bIsActivated)
    {
        ActivateLayer();
    }
    else
    {
        DeactivateLayer();
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (ULSUISubsystem* Subsystem = GI->GetSubsystem<ULSUISubsystem>())
        {
            CachedUISubsystem = Subsystem;
        }
    }
}
