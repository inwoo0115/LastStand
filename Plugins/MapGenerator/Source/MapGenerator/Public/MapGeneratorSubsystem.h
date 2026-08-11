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

	// 후처리 1: 8방향 극댓값을 찾아 보로노이 시드 포인트로 지정(인접 플래토는 첫 포인트로 병합).
	//           Grid.VoronoiPoints를 채우고 시드 셀에 RegionIndices 초기 라벨 부여
	void FindLocalMaxima(FMapGrid& Grid) const;

	// 후처리 2: JFA(Jump Flooding)로 각 셀을 최근접 시드 포인트 영역으로 라벨링 → Grid.RegionIndices 완성
	void BuildVoronoiRegions(FMapGrid& Grid) const;

	// 후처리 3: 서로 다른 영역이 맞닿는 경계 셀(+주변 Thickness)을 경계로 표시 → 높이 0, RegionIndices = -2
	void MarkRegionBoundaries(FMapGrid& Grid, int32 Thickness) const;

	// 후처리 4: 높이 값을 Step 간격 최근접 배수로 반올림 후 [-1,1] 클램프 (Step<=0이면 무시)
	void QuantizeHeights(FMapGrid& Grid, float Step) const;

protected:
	// FMapAssetData 행 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> MapAssetTable;

	// FMapData 행 테이블 (맵 생성/노이즈 파라미터)
	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;

};
