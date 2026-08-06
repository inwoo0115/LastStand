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
#include "GameFramework/Pawn.h"
#include "Character/Components/LSEquipmentComponent.h"

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
    Op->ItemID = ItemID;              // 언이큅 대상 식별
    Op->Quantity = Quantity;
    Op->DefaultDragVisual = this;     // 끌 때 보일 비주얼
    Op->Pivot = EDragPivot::MouseDown;
    OutOperation = Op;
}

FReply ULSEquipmentEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 오른쪽 버튼: 장비칸 아이템을 인벤토리로 되돌림
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        if (APawn* P = GetOwningPlayerPawn())
        {
            if (ULSEquipmentComponent* Equip = P->FindComponentByClass<ULSEquipmentComponent>())
            {
                Equip->ServerRPCUnEquipItemFromInventory(ItemID);
            }
        }
        return FReply::Handled();
    }

    // 왼쪽 버튼이 눌리면 드래그 감지
    return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}
