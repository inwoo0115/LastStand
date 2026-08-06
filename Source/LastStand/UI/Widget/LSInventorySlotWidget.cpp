// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSInventorySlotWidget.h"
#include "LSInventorySlotWidget.h"
#include "UI/Operation/LSItemDragDropOperation.h"
#include "UI/Operation/LSEquipmentDragDropOperation.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Character/Components/LSInventoryComponent.h"
#include "Character/Components/LSInteractionComponent.h"
#include "Character/Components/LSEquipmentComponent.h"
#include "LSInventoryEntryWidget.h"

void ULSInventorySlotWidget::RefreshSlot()
{
	if (!InventoryComp.IsValid() || !Container)
	{
		return;
	}

	if (!EntryWidgetClass)
	{
		return;
	}


	Container->ClearChildren();

	// InventoryContainer 초기화
	const FInventoryItemInfoArray& InfoArray = InventoryComp->GetInventoryItems();

	FGuid EmptyGuid(0, 0, 0, 0);

	for (const FInventoryItemInfo& Info : InfoArray.Items)
	{
		ULSInventoryEntryWidget* Entry = CreateWidget<ULSInventoryEntryWidget>(this, EntryWidgetClass);
		Entry->SetItem(Info.ItemID, Info.Quantity, true, EmptyGuid);
		Container->AddChildToVerticalBox(Entry);
	}

	// 드래그 중인 위젯 취소 TODO: 드래그를 강제 취소말고 다른 방법을 생각해 보긴 해야할듯?
	if (FSlateApplication::IsInitialized() && FSlateApplication::Get().IsDragDropping())
	{
		FSlateApplication::Get().CancelDragDrop();
	}
}

void ULSInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APawn* P = GetOwningPlayerPawn())
	{
		InventoryComp = P->FindComponentByClass<ULSInventoryComponent>();
		if (InventoryComp.IsValid())
		{
			InventoryComp->OnInventoryUpdated.AddUObject(this, &ULSInventorySlotWidget::RefreshSlot);

			RefreshSlot();   // 최초 1회
		}

		InteractionComp = P->FindComponentByClass<ULSInteractionComponent>();
		if (!InteractionComp.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("Interaction Comp is not valid"));
		}

		EquipmentComp = P->FindComponentByClass<ULSEquipmentComponent>();
		if (!EquipmentComp.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("Equipment Comp is not valid"));
		}
	}
}

void ULSInventorySlotWidget::NativeDestruct()
{
	// 델리게이트 해제
	if (InventoryComp.IsValid())
	{
		InventoryComp->OnInventoryUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

bool ULSInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 장비칸에서 끌어온 아이템 → 언이큅(인벤토리로 복귀)
	if (ULSEquipmentDragDropOperation* EquipOp = Cast<ULSEquipmentDragDropOperation>(InOperation))
	{
		if (!EquipmentComp.IsValid())
		{
			return false;
		}
		EquipmentComp->ServerRPCUnEquipItemFromInventory(EquipOp->ItemID);
		return true;
	}

	// 월드 후보 → 인벤토리 픽업(널가드 추가)
	ULSItemDragDropOperation* ItemOp = Cast<ULSItemDragDropOperation>(InOperation);
	if (!ItemOp || ItemOp->bFromInventory)
	{
		return false;
	}

	if (!InteractionComp.IsValid())
	{
		return false;
	}

	// 페이로드로 받은 데이터로 아이템을 인벤토리 추가
	InteractionComp->ServerRPCInteractCertainCandidate(ItemOp->InstanceID);


	return true;
}

bool ULSInventorySlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return true;
}
