// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/LSWeaponInfoData.h"
#include "LSWeaponData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TObjectPtr<ULSWeaponInfoData> WeaponDataAsset;
};