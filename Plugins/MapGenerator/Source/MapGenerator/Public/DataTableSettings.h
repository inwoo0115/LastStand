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
	// FMapAssetData 행 테이블 (WFC 등에 쓰일 맵 에셋)
	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> MapAssetTable;

	// FMapData 행 테이블 (맵 생성/노이즈 파라미터)
	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> MapDataTable;

};
