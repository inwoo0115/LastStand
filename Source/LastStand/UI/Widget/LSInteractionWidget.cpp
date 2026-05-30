// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSInteractionWidget.h"
#include "Character/Components/LSInteractionComponent.h"
#include "Components/TextBlock.h"
#include "LSInteractionWidget.h"

void ULSInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();

    AActor* OwnerActor = GetOwningPlayerPawn();
    if (OwnerActor)
    {
        ULSInteractionComponent* Comp = OwnerActor->FindComponentByClass<ULSInteractionComponent>();
        if (Comp)
        {
            Comp->OnInteractionUpdate.AddUObject(this, &ULSInteractionWidget::HandleNameUpdate);
        }
    }

    SetVisibility(ESlateVisibility::Hidden);
}

void ULSInteractionWidget::HandleNameUpdate(FName NewName)
{
    if (NewName == NAME_None)
    {
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }
    else
    {
        SetVisibility(ESlateVisibility::Visible);
    }

    if (NameTextBlock)
    {
        NameTextBlock->SetText(FText::FromName(NewName));
    }
}
