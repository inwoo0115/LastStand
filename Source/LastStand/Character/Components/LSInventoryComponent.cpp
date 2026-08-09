// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSInventoryComponent.h"
#include "Net/UnrealNetwork.h" 
#include "Item/LSItemArray.h"
#include "DataTable/LSDataSubsystem.h"
#include "Props/LSDropItem.h"
#include "Kismet/GameplayStatics.h"


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

	// 서버측 게임플레이 연동 (탄알 캐싱 등) — 습득은 양수 델타
	OnInventoryItemChanged.Broadcast(ItemInfoID, Quantity);
}

void ULSInventoryComponent::RemoveItemFromInventory(const FName ItemInfoID)
{
	InventoryItems.RemoveInventoryItem(ItemInfoID);
}

void ULSInventoryComponent::UpdateItemInInventory(const FName ItemInfoID, const int32 NewQuantity)
{
	if (!InventoryItems.UpdateItemQuantity(ItemInfoID, NewQuantity))
	{
		UE_LOG(LogTemp, Log, TEXT("UpdateItemInInventory"));
	}
}

void ULSInventoryComponent::AddDeltaToItem(const FName ItemInfoID, const int32 Delta)
{
	// 실제 수량 변경이 일어났을 때만 게임플레이 연동 브로드캐스트 (탄알 캐시 동기화 등)
	if (InventoryItems.AddItemQuantity(ItemInfoID, Delta))
	{
		OnInventoryItemChanged.Broadcast(ItemInfoID, Delta);
	}
}

const FInventoryItemInfoArray& ULSInventoryComponent::GetInventoryItems() const
{
	return InventoryItems;
}


void ULSInventoryComponent::ServerRPCAddItemToInventory_Implementation(const FName ItemInfoID, const int32 Quantity)
{
	// 습득 경로를 단일화 (UI 갱신 + 게임플레이 델리게이트 브로드캐스트 공유)
	AddItemToInventory(ItemInfoID, Quantity);
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

		// 지연 스폰: BeginPlay 전에 ItemName/Quantity 주입
		const FTransform SpawnTransform(FRotator::ZeroRotator, Loc);
		ALSDropItem* Drop = GetWorld()->SpawnActorDeferred<ALSDropItem>(DropClass, SpawnTransform, GetOwner());
		if (Drop)
		{
			Drop->InitItem(ItemID, Quantity);
			UGameplayStatics::FinishSpawningActor(Drop, SpawnTransform);
		}
	}

	AddDeltaToItem(ItemID, -Quantity);
}



