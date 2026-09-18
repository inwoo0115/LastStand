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

#if WITH_EDITOR
	// MapDataTable 전 행을 순회해 각 방 레벨의 ABoxVolume 바운즈를 행(BoundsBoxes)에 export
	UFUNCTION(CallInEditor, Category = "Tools")
	void ExportRoomBounds();
#endif

};
