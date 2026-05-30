// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSInventoryComponent.h"
#include "Net/UnrealNetwork.h" 
#include "Item/LSItemArray.h"


ULSInventoryComponent::ULSInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void ULSInventoryComponent::AddItemToInventory(const FName ItemInfoID, const int32 Quantity)
{
	FInventoryItemInfo NewInfo(ItemInfoID, Quantity);

	InventoryItems.AddInventoryItem(NewInfo);
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


void ULSInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULSInventoryComponent, InventoryItems);
}



