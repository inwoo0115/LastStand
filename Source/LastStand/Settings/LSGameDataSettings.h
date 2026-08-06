// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LSGameDataSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Game Data"))
class LASTSTAND_API ULSGameDataSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> ItemTable;

	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> WeaponTable;

	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> EnemyTable;
};
