#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LSItemArray.generated.h"

USTRUCT()
struct FInventoryItemInfo : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventoryItemInfo() {};

	// Array에 저장할 요소
	UPROPERTY()
	int32 ItemID;
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
		int32 Index = Items.Add(NewItemInfo);
		MarkItemDirty(Items[Index]);
		Items[Index].PostReplicatedAdd(*this);
	};

	void RemoveInventoryItem(const int32 ItemInfoID)
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
