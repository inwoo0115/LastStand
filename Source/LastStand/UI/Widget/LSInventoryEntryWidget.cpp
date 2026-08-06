// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSInventoryEntryWidget.h"
#include "DataTable/LSDataSubsystem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/Operation/LSItemDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Input/Reply.h"
#include "GameFramework/Pawn.h"
#include "Character/Components/LSEquipmentComponent.h"
#include "Character/Components/LSInteractionComponent.h"

void ULSInventoryEntryWidget::SetItem(FName InItemID, int32 InQuantity, bool bInFromInventory, FGuid InInstanceID)
{
	ItemID = InItemID; 
	Quantity = InQuantity; 
	bFromInventory = bInFromInventory;
    InstanceID = InInstanceID;

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
    NameText->SetText(FText::FromName(ItemID));
}

void ULSInventoryEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    ULSItemDragDropOperation* Op = NewObject<ULSItemDragDropOperation>();
    Op->ItemID = ItemID;
    Op->Quantity = Quantity;
    Op->bFromInventory = bFromInventory;
    Op->InstanceID = InstanceID;      // 월드 후보 식별용 GUID 전달
    Op->DefaultDragVisual = this;     // 끌 때 보일 비주얼 (간단히 자기 자신)
    Op->Pivot = EDragPivot::MouseDown;
    Op->InstanceID = InstanceID;
    OutOperation = Op;
}

FReply ULSInventoryEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 오른쪽 버튼: 드래그와 유사한 이동을 즉시 실행
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        HandleRightClick();
        return FReply::Handled();
    }

    // 왼쪽 버튼이 눌리면 드래그 감지
    return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void ULSInventoryEntryWidget::HandleRightClick()
{
    APawn* P = GetOwningPlayerPawn();
    if (!P)
    {
        return;
    }

    if (bFromInventory)
    {
        // 인벤토리 아이템 → 타입에 맞는 장비칸(차 있으면 드래그와 동일하게 스왑)
        if (ULSEquipmentComponent* Equip = P->FindComponentByClass<ULSEquipmentComponent>())
        {
            Equip->ServerRPCEquipItemFromInventory(ItemID);
        }
    }
    else
    {
        // 월드 후보(Interaction) → 인벤토리 픽업
        if (ULSInteractionComponent* Interaction = P->FindComponentByClass<ULSInteractionComponent>())
        {
            Interaction->ServerRPCInteractCertainCandidate(InstanceID);
        }
    }
}
