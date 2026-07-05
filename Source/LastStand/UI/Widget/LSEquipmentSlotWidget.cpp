// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSEquipmentSlotWidget.h"
#include "LSEquipmentSlotWidget.h"
#include "UI/Operation/LSItemDragDropOperation.h"
#include "Character/Components/LSEquipmentComponent.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "UI/Widget/LSEquipmentEntryWidget.h"
#include "Item/Equipment/LSEquipmentBase.h"

void ULSEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APawn* P = GetOwningPlayerPawn())
	{
		EquipmentComp = P->FindComponentByClass<ULSEquipmentComponent>();
		if (EquipmentComp.IsValid())
		{
			EquipmentComp->OnEquipmentArrayUpdated.AddUObject(this, &ULSEquipmentSlotWidget::RefreshSlot);

			RefreshSlot();
		}
	}
}

void ULSEquipmentSlotWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

bool ULSEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	ULSItemDragDropOperation* ItemOp = Cast<ULSItemDragDropOperation>(InOperation);
	
	if (!ItemOp->bFromInventory)
	{
		return false;
	}

	if (!EquipmentComp.IsValid())
	{
		return false;
	}

	EquipmentComp->ServerRPCEquipItemFromInventory(ItemOp->ItemID);

	return true;
}

bool ULSEquipmentSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return true;
}

void ULSEquipmentSlotWidget::RefreshSlot()
{
	UE_LOG(LogTemp, Log, TEXT("ULSEquipmentSlotWidget::RefreshSlot()"));

	if (!Container || !EquipmentComp.IsValid())
	{
		return;
	}

	if (!EntryWidgetClass)
	{
		return;
	}

	Container->ClearChildren();

	const TMap<EEquipmentType, TObjectPtr<AActor>> EquipmentMap = EquipmentComp->GetEquipments();
	if (!EquipmentMap.IsEmpty())
	{
		if (EquipmentMap.Find(EquipmentType))
		{
			UE_LOG(LogTemp, Log, TEXT("ULSEquipmentSlotWidget: Create Equipment Entry Widget"));
			ALSEquipmentBase* EB = Cast<ALSEquipmentBase>(EquipmentMap[EquipmentType]);
			if (EB)
			{
				ULSEquipmentEntryWidget* Entry = CreateWidget<ULSEquipmentEntryWidget>(this, EntryWidgetClass);
				if (Entry)
				{
					Container->AddChild(Entry);
					Entry->SetEntry(EB->GetItemName());
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentSlotWidget: EquipmentMap is Empty"));
	}

	// 드래그 중인 위젯 취소
	if (FSlateApplication::IsInitialized() && FSlateApplication::Get().IsDragDropping())
	{
		FSlateApplication::Get().CancelDragDrop();
	}
}