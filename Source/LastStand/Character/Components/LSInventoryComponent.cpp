// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSInventoryComponent.h"
#include "Net/UnrealNetwork.h" 
#include "Item/LSItemArray.h"
#include "DataTable/LSDataSubsystem.h"
#include "Props/LSDropItem.h"


ULSInventoryComponent::ULSInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void ULSInventoryComponent::AddItemToInventory(const FName ItemInfoID, const int32 Quantity)
{
	FInventoryItemInfo NewInfo(ItemInfoID, Quantity);

	InventoryItems.AddInventoryItem(NewInfo);

	// authority(호스트/스탠드얼론)는 RepNotify가 호출되지 않으므로 직접 UI 갱신
	OnInventoryItemChange();
}

void ULSInventoryComponent::RemoveItemFromInventory(const FName ItemInfoID)
{
	InventoryItems.RemoveInventoryItem(ItemInfoID);
}

void ULSInventoryComponent::UpdateItemInInventory(const FName ItemInfoID, const int32 NewQuantity)
{
	if (!InventoryItems.UpdateItemQuantity(ItemInfoID, NewQuantity))
	{
		UE_LOG(LogTemp, Log, TEXT("No available Item in Inventory"));
	}
}

void ULSInventoryComponent::AddDeltaToItem(const FName ItemInfoID, const int32 Delta)
{
	if (InventoryItems.AddItemQuantity(ItemInfoID, Delta))
	{
		UE_LOG(LogTemp, Log, TEXT("No available Item in Inventory"));
	}
}

const FInventoryItemInfoArray& ULSInventoryComponent::GetInventoryItems() const
{
	return InventoryItems;
}


void ULSInventoryComponent::ServerRPCAddItemToInventory_Implementation(const FName ItemInfoID, const int32 Quantity)
{
	FInventoryItemInfo NewInfo(ItemInfoID, Quantity);

	InventoryItems.AddInventoryItem(NewInfo);
}

void ULSInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULSInventoryComponent, InventoryItems);
}

void ULSInventoryComponent::OnInventoryItemChange()
{
	// UI 변경
	OnInventoryUpdated.Broadcast();
}

void ULSInventoryComponent::ServerRPCDropItemToWorld_Implementation(FName ItemID, int32 Quantity)
{
	UE_LOG(LogTemp, Log, TEXT("ServerRPCDropItemToWorld_Implementation"));

	// Data 검색
	const ULSDataSubsystem* Data = GetWorld()->GetGameInstance()->GetSubsystem<ULSDataSubsystem>();
	if (!Data)
	{
		UE_LOG(LogTemp, Log, TEXT("Can not found ULSDataSubsystem"));
		return;
	}

	const FItemData* Row = Data ? Data->FindItem(ItemID) : nullptr;
	if (!Row)
	{
		UE_LOG(LogTemp, Log, TEXT("Can not found Item: %s"), *ItemID.ToString());
		return;
	}

	if (UClass* DropClass = Row->DropItemClass.LoadSynchronous())
	{
		UE_LOG(LogTemp, Log, TEXT("Drop Item Class Loading"));

		const FVector Loc = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 150.f;
		ALSDropItem* Drop = GetWorld()->SpawnActor<ALSDropItem>(DropClass, Loc, FRotator::ZeroRotator);
		Drop->InitItem(ItemID, Quantity);
	}

	AddDeltaToItem(ItemID, -Quantity);
}



