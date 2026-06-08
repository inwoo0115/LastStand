// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LSSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
    UPROPERTY(VisibleAnywhere, Category = SaveGame)
    FString PlayerName = "Default";

    UPROPERTY(VisibleAnywhere, Category = SaveGame)
    FString SaveSlotName = "TestSlot";

    UPROPERTY(VisibleAnywhere, Category = SaveGame)
    uint32 PlayerIndex = 0;

    UPROPERTY(VisibleAnywhere, Category = SaveGame)
    int32 SaveVersion = 1;   // 포맷 변경 시 마이그레이션용

    UPROPERTY(VisibleAnywhere, Category = SaveGame)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, Category = SaveGame)
    FName LastLevel = "";

    UPROPERTY(VisibleAnywhere, Category = SaveGame)
    float Health = 100.f;

    UPROPERTY(VisibleAnywhere, Category = SaveGame)
    TArray<FName> InventoryItems;
	
	
};
