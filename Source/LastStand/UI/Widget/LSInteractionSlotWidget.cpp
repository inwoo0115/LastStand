// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSInteractionSlotWidget.h"
#include "UI/Operation/LSItemDragDropOperation.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Character/Components/LSInteractionComponent.h"
#include "Character/Components/LSInventoryComponent.h"
#include "LSInventoryEntryWidget.h"
#include "Props/LSDropItem.h"

void ULSInteractionSlotWidget::RefreshSlot()
{
	if (!InteractionComp.IsValid() || !Container)
	{
		return;
	}

	if (!EntryWidgetClass)
	{
		return;
	}

	Container->ClearChildren();

	if (!InteractionComp->CanInteract())
	{
		return;
	}

	// InteractionContainer 초기화
	const TArray<TObjectPtr<AActor>>& InfoArray = InteractionComp->GetCandidates();

	for (const TObjectPtr<AActor>& Info : InfoArray)
	{
		ALSDropItem* DI = Cast<ALSDropItem>(Info);
		if (!DI)
		{
			continue;
		}
		ULSInventoryEntryWidget* Entry = CreateWidget<ULSInventoryEntryWidget>(this, EntryWidgetClass);
		if (Entry)
		{
			Container->AddChildToVerticalBox(Entry);
			Entry->SetItem(DI->GetItemName(), DI->GetQuantity(), false, DI->GetInstanceID());
		}
	}

	// 드래그 중인 위젯 취소
	if (FSlateApplication::IsInitialized() && FSlateApplication::Get().IsDragDropping())
	{
		FSlateApplication::Get().CancelDragDrop();
	}
}

void ULSInteractionSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APawn* P = GetOwningPlayerPawn())
	{
		InteractionComp = P->FindComponentByClass<ULSInteractionComponent>();
		if (InteractionComp.IsValid())
		{
			InteractionComp->OnInteractionArrayUpdate.AddUObject(this, &ULSInteractionSlotWidget::RefreshSlot);

			RefreshSlot();   // 최초 1회
		}

		InventoryComp = P->FindComponentByClass<ULSInventoryComponent>();
		if (!InventoryComp.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("Inventory Comp is not valid"));
		}
	}
}

void ULSInteractionSlotWidget::NativeDestruct()
{
	// 델리게이트 해제
	if (InteractionComp.IsValid())
	{
		InteractionComp->OnInteractionArrayUpdate.RemoveAll(this);
	}

	Super::NativeDestruct();
}

bool ULSInteractionSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	ULSItemDragDropOperation* ItemOp = Cast<ULSItemDragDropOperation>(InOperation);

	// 받은 데이터 기반으로 드랍 이벤트 처리
	if (!ItemOp->bFromInventory)
	{
		return false;
	}

	if (!InventoryComp.IsValid())
	{
		return false;
	}

	// 페이로드로 받은 데이터로 아이템을 스폰
	InventoryComp->ServerRPCDropItemToWorld(ItemOp->ItemID, ItemOp->Quantity);

	return true;
}

bool ULSInteractionSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return true;
}