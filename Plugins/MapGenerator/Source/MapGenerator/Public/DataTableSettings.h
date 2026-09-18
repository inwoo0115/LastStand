// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DataTableSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Map Data"))
class MAPGENERATOR_API UDataTableSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> MapDataTable;

};
