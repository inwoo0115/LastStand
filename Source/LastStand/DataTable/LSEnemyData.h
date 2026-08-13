// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LSEnemyData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FName EnemyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 MaxHealth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 CurrentHealth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 AttackDamage = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    TSoftClassPtr<APawn> EnemyClass;
};