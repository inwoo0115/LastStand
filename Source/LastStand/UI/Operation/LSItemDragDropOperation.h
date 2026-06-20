// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "LSItemDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FName ItemID;
	UPROPERTY() 
	int32 Quantity = 0;

	UPROPERTY()
	FGuid InstanceID;


	// 출처 구분: 인벤토리에서 끌었나, 월드 후보에서 끌었나
	UPROPERTY() bool bFromInventory = false;
};
