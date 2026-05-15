// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LSRootLayoutWidget.h"
#include "UI/LSUISubsystem.h"
#include "Tags/LSGameplayTags.h"

void ULSRootLayoutWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UGameInstance* GI = GetGameInstance())
    {
        if (ULSUISubsystem* Subsystem = GI->GetSubsystem<ULSUISubsystem>())
        {
            Subsystem->RegisterLayer(LSUITags::Layer_Game, LayerGame);
            Subsystem->RegisterLayer(LSUITags::Layer_Menu, LayerMenu);
            Subsystem->RegisterLayer(LSUITags::Layer_Modal, LayerModal);
        }
    }
}
