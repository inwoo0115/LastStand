// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSEquipmentEntryWidget.h"
#include "LSEquipmentEntryWidget.h"
#include "UI/Operation/LSEquipmentDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Input/Reply.h"
#include "DataTable/LSDataSubsystem.h"
#include "DataTable/LSWeaponData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void ULSEquipmentEntryWidget::SetEntry(FName InItemID)
{
    ItemID = InItemID;

    // DataSubsystem으로 아이콘 로드
    if (const ULSDataSubsystem* Data = GetGameInstance()->GetSubsystem<ULSDataSubsystem>())
    {
        if (const FWeaponData* Row = Data->FindWeapon(ItemID))
        {
            if (UTexture2D* Tex = Row->Icon.LoadSynchronous())
            {
                IconImage->SetBrushFromTexture(Tex);
            }
        }
    }
    NameText->SetText(FText::FromName(ItemID));
}

void ULSEquipmentEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    ULSEquipmentDragDropOperation* Op = NewObject<ULSEquipmentDragDropOperation>();

    // Inventory에 전달할 데이터 TODO: 추후 구현


    OutOperation = Op;
}

FReply ULSEquipmentEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 왼쪽 버튼이 눌리면 드래그 감지
    return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}
