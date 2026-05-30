#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LSItemArray.generated.h"

USTRUCT()
struct FInventoryItemInfo : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventoryItemInfo() {};

	FInventoryItemInfo(FName NewID, int32 NewQuantity) : ItemID(NewID), Quantity(NewQuantity) {};

	// Array에 저장할 요소
	UPROPERTY()
	FName ItemID;

	UPROPERTY()
	int32 Quantity = 0;
};

USTRUCT()
struct FInventoryItemInfoArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FInventoryItemInfo> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FInventoryItemInfo, FInventoryItemInfoArray>(Items, DeltaParms, *this);
	};

	void AddInventoryItem(const FInventoryItemInfo& NewItemInfo) 
	{
		// 이미 같은 ItemID가 있으면 수량만 합산
		for (FInventoryItemInfo& Item : Items)
		{
			if (Item.ItemID == NewItemInfo.ItemID)
			{
				Item.Quantity += NewItemInfo.Quantity;
				MarkItemDirty(Item);
				return;
			}
		}

		// 없으면 새 엔트리 추가
		const int32 Index = Items.Add(NewItemInfo);
		MarkItemDirty(Items[Index]);
	};

	bool UpdateItemQuantity(const FName ItemID, int32 NewQuantity)
	{
		for (FInventoryItemInfo& Item : Items)
		{
			if (Item.ItemID == ItemID)
			{
				Item.Quantity = NewQuantity;
				MarkItemDirty(Item);
				return true;
			}
		}
		return false;
	}

	bool AddItemQuantity(const FName ItemID, int32 Delta)
	{
		for (FInventoryItemInfo& Item : Items)
		{
			if (Item.ItemID == ItemID)
			{
				Item.Quantity += Delta;
				if (Item.Quantity <= 0)
				{
					RemoveInventoryItem(ItemID);
				}
				else
				{
					MarkItemDirty(Item);
				}
				return true;
			}
		}
		return false;
	}

	void RemoveInventoryItem(const FName ItemInfoID)
	{
		for (int32 Index = 0; Index < Items.Num(); ++Index)
		{
			FInventoryItemInfo& InventoryInfo = Items[Index];
			if (ItemInfoID == InventoryInfo.ItemID)
			{
				InventoryInfo.PreReplicatedRemove(*this);
				Items.RemoveAtSwap(Index);
				MarkArrayDirty();
				break;
			}
		}
	};
};

template<>
struct TStructOpsTypeTraits<FInventoryItemInfoArray> : public TStructOpsTypeTraitsBase2<FInventoryItemInfoArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
