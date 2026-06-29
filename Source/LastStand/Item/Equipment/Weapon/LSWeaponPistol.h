// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "LSWeaponPistol.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSWeaponPistol : public ALSWeaponBase
{
	GENERATED_BODY()
	
public:
	virtual void LaunchWeapon() override;

	virtual void ReleaseWeapon() override;

	virtual void Equipped() override;

	virtual void UnEquipped() override;
	

};
