// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LSItemData.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Weapon,
    Armor,
    Consumable,
    Quest,
    Ammo
};

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemType ItemType = EItemType::Consumable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float Weight = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Value = 0;

    // 아이콘 에셋은 소프트 레퍼런스로 — 하드 레퍼런스 금지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSoftClassPtr<AActor> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSoftClassPtr<AActor> DropItemClass;
};