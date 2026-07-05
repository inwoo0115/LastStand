// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSEquipmentEntryWidget.h"
#include "LSEquipmentEntryWidget.h"
#include "UI/Operation/LSEquipmentDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Input/Reply.h"

void ULSEquipmentEntryWidget::SetEntry(FName InItemID)
{
    // Data subsystem에서 무기 정보 찾아서 이미지 세팅
}

void ULSEquipmentEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    ULSEquipmentDragDropOperation* Op = NewObject<ULSEquipmentDragDropOperation>();

    // Inventory에 전달할 데이터


    OutOperation = Op;
}

FReply ULSEquipmentEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 왼쪽 버튼이 눌리면 드래그 감지
    return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}
