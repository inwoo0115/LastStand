// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/LSItemBase.h"
#include "LSEquipmentBase.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSEquipmentBase : public ALSItemBase
{
	GENERATED_BODY()
	
public:
	virtual void Equipped();

	virtual void UnEquipped();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void InitEquipment();
};
