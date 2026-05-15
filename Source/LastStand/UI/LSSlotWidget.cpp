// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LSSlotWidget.h"
#include "LSUISubsystem.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void ULSSlotWidget::AddWidget(TSubclassOf<UUserWidget> WidgetClass, int32 HandleId)
{
    if (!WidgetClass || !Container) return;
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (!NewWidget) return;
    Container->AddChildToVerticalBox(NewWidget);
    InjectedWidgets.Add(HandleId, NewWidget);
}

void ULSSlotWidget::RemoveWidget(int32 HandleId)
{
    if (UUserWidget* Widget = InjectedWidgets.FindRef(HandleId))
    {
        Container->RemoveChild(Widget);
        InjectedWidgets.Remove(HandleId);
    }
}

void ULSSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ensureMsgf(SlotTag.IsValid(), TEXT("LSSlotWidget: SlotTag is not set!"));
    if (UGameInstance* GI = GetGameInstance())
    {
        if (ULSUISubsystem* Manager = GI->GetSubsystem<ULSUISubsystem>())
        {
            Manager->RegisterSlot(SlotTag, this);
        }
    }
}

void ULSSlotWidget::NativeDestruct()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (ULSUISubsystem* Manager = GI->GetSubsystem<ULSUISubsystem>())
        {
            Manager->UnregisterSlot(SlotTag, this);
        }
    }
    Super::NativeDestruct();
}
