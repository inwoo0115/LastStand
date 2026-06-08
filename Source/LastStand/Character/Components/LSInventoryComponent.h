// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/LSItemArray.h"
#include "LSInventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTSTAND_API ULSInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULSInventoryComponent();

	void AddItemToInventory(const FName ItemInfoID, const int32 Quantity);

	void RemoveItemFromInventory(const FName ItemInfoID);

	void UpdateItemInInventory(const FName ItemInfoID, const int32 NewQuantity);

	void AddDeltaToItem(const FName ItemInfoID, const int32 Delta);

	const FInventoryItemInfoArray& GetInventoryItems() const;

	void DropItemToWorld(FName ItemID, int32 Quantity);

	// UI Delegate
	FOnInventoryUpdated OnInventoryUpdated;
protected:
	// RPC
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Fast Array Sericalizer 아이템 배열
	UPROPERTY(ReplicatedUsing=OnInventoryItemChange)
	FInventoryItemInfoArray InventoryItems;

	UFUNCTION()
	void OnInventoryItemChange();

	UFUNCTION(Server, Reliable)
	void ServerRPCDropItemToWorld(FName ItemID, int32 Quantity);
	
};
