// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSEquipmentSlotWidget.h"
#include "LSEquipmentSlotWidget.h"
#include "UI/Operation/LSItemDragDropOperation.h"

void ULSEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULSEquipmentSlotWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

bool ULSEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	ULSItemDragDropOperation* ItemOp = Cast<ULSItemDragDropOperation>(InOperation);

	// 장비 인지 체크

	// Equipment Component 확인

	// interact에서 드래그 시 처리

	// inventroy 드래그 시 처리

	// 일단 해제는 나중에 만들어야 할듯 함
	return true;
}

bool ULSEquipmentSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return true;
}
