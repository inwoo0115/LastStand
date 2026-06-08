// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSInventoryEntryWidget.h"
#include "DataTable/LSDataSubsystem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/Operation/LSItemDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Input/Reply.h"

void ULSInventoryEntryWidget::SetItem(FName InItemID, int32 InQuantity, bool bInFromInventory)
{
	ItemID = InItemID; 
	Quantity = InQuantity; 
	bFromInventory = bInFromInventory;

    // DataSubsystem으로 아이콘 로드
    if (const ULSDataSubsystem* Data = GetGameInstance()->GetSubsystem<ULSDataSubsystem>())
    {
        if (const FItemData* Row = Data->FindItem(ItemID))
        {
            if (UTexture2D* Tex = Row->Icon.LoadSynchronous())
            {
                IconImage->SetBrushFromTexture(Tex);
            }
        }
    }
    QuantityText->SetText(FText::AsNumber(Quantity));
}

void ULSInventoryEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    ULSItemDragDropOperation* Op = NewObject<ULSItemDragDropOperation>();
    Op->ItemID = ItemID; 
    Op->Quantity = Quantity; 
    Op->bFromInventory = bFromInventory;
    Op->DefaultDragVisual = this;     // 끌 때 보일 비주얼 (간단히 자기 자신)
    Op->Pivot = EDragPivot::MouseDown;
    OutOperation = Op;
}

FReply ULSInventoryEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 왼쪽 버튼이 눌리면 드래그 감지
    return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}
