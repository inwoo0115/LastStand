// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSInteractionSlotWidget.h"
#include "UI/Operation/LSItemDragDropOperation.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Character/Components/LSInteractionComponent.h"
#include "LSInventoryEntryWidget.h"
#include "Props/LSDropItem.h"

void ULSInteractionSlotWidget::RefreshSlot()
{
	if (!InteractionComp.IsValid() || !Container)
	{
		return;
	}

	Container->ClearChildren();

	// InteractionContainer 초기화
	const TArray<TObjectPtr<AActor>> InfoArray = InteractionComp->GetCandidates();

	for (const TObjectPtr<AActor> Info : InfoArray)
	{
		const TObjectPtr<ALSDropItem> DI = Cast<ALSDropItem>(Info);
		if (!DI)
		{
			continue;
		}
		ULSInventoryEntryWidget* Entry = CreateWidget<ULSInventoryEntryWidget>(this, EntryWidgetClass);
		Entry->SetItem(DI->GetItemName(), DI->GetQuantity(), false);
		Container->AddChildToVerticalBox(Entry);
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

	// 페이로드로 받은 데이터로 아이템을 스폰

	// 인벤토리에서 제거 
	


	return true;
}

bool ULSInteractionSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return true;
}