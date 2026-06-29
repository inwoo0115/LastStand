// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equipment/LSEquipmentBase.h"
#include "DataTable/LSItemData.h"
#include "LSWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSWeaponBase : public ALSEquipmentBase
{
	GENERATED_BODY()

public:
	virtual void LaunchWeapon();

	virtual void ReleaseWeapon();
	
	virtual void Equipped() override;

	virtual void UnEquipped() override;

	virtual void ReloadWeapon();

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void InitEquipment() override;

	UPROPERTY()
	FItemData ItemData;
};
