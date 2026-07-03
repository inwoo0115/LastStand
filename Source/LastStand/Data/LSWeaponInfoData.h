// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LSWeaponInfoData.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSWeaponInfoData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = Bullet)
	uint32 MaxAmmo;
	
	UPROPERTY(EditAnywhere, Category = SpringArm)
	float TargetArmLength = 40.0f;
};
