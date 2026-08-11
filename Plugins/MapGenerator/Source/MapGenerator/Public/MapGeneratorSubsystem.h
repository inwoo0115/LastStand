// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MapAssetData.h"
#include "MapData.h"
#include "MapGrid.h"
#include "MapGeneratorSubsystem.generated.h"

/**
 * 맵 생성 서브시스템. 레벨(월드)마다 독립적으로 관리.
 */
UCLASS()
class MAPGENERATOR_API UMapGeneratorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;


	const FMapAssetData* FindAsset(FName AssetID) const;

	// MapData 테이블에서 행 조회 (RowName 기준)
	const FMapData* FindMapData(FName MapName) const;

	// MapName 행의 파라미터로 펄린 노이즈(fBm) 높이 그리드 생성
	UFUNCTION(BlueprintCallable, Category = "MapGenerator")
	FMapGrid GenerateHeightGrid(FName MapName);

protected:
	// FMapAssetData 행 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> MapAssetTable;

	// FMapData 행 테이블 (맵 생성/노이즈 파라미터)
	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;

};
