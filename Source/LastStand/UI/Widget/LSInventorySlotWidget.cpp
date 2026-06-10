// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSInventorySlotWidget.h"
#include "LSInventorySlotWidget.h"
#include "UI/Operation/LSItemDragDropOperation.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Character/Components/LSInventoryComponent.h"
#include "LSInventoryEntryWidget.h"

void ULSInventorySlotWidget::RefreshSlot()
{
	if (!InventoryComp.IsValid() || !Container)
	{
		return;
	}

	Container->ClearChildren();

	// InventoryContainer 초기화
	const FInventoryItemInfoArray& InfoArray = InventoryComp->GetInventoryItems();

	for (const FInventoryItemInfo& Info : InfoArray.Items)
	{
		ULSInventoryEntryWidget* Entry = CreateWidget<ULSInventoryEntryWidget>(this, EntryWidgetClass);
		Entry->SetItem(Info.ItemID, Info.Quantity, true);
		Container->AddChildToVerticalBox(Entry);
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
	ULSItemDragDropOperation* ItemOp = Cast<ULSItemDragDropOperation>(InOperation);
	
	// 받은 데이터 기반으로 드랍 이벤트 처리

	// 페이로드로 받은 데이터로 아이템을 인벤토리 추가

	// 필드에 있는 오브젝트 제거

	return true;
}

bool ULSInventorySlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return true;
}
