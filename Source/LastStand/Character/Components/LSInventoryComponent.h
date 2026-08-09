// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/LSItemArray.h"
#include "LSInventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

// 서버 권위 경로에서 인벤토리 아이템 수량이 변할 때 발화 (Delta 부호 있음). 탄알 캐싱 등 게임플레이 연동용
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemChanged, FName /*ItemID*/, int32 /*Delta*/);

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

	// UI Delegate
	FOnInventoryUpdated OnInventoryUpdated;

	// 서버측 게임플레이 델리게이트 (아이템 수량 변경 시, Delta 부호 있음)
	FOnInventoryItemChanged OnInventoryItemChanged;
	
	// Server RPC
	UFUNCTION(Server, Reliable)
	void ServerRPCDropItemToWorld(FName ItemID, int32 Quantity);

	UFUNCTION(Server, Reliable)
	void ServerRPCAddItemToInventory(const FName ItemInfoID, const int32 Quantity);

protected:
	// RPC
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Fast Array Sericalizer 아이템 배열
	UPROPERTY(ReplicatedUsing=OnInventoryItemChange)
	FInventoryItemInfoArray InventoryItems;

	UFUNCTION()
	void OnInventoryItemChange();
};
