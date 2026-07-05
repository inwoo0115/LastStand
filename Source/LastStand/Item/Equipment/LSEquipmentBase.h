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
	ALSEquipmentBase();

	virtual void Equipped();

	virtual void UnEquipped();

	virtual void ActivateEquipment();

	virtual void DeActivateEquipment();

	
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USceneComponent> Root;

	virtual void BeginPlay() override;
	
	virtual void InitEquipment();
};
