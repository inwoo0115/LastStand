// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MapAssetData.h"
#include "MapData.h"
#include "MapWFCData.h"
#include "MapGrid.h"
#include "MapTileGrid.h"
#include "MapGeneratorSubsystem.generated.h"

UCLASS()
class MAPGENERATOR_API UMapGeneratorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

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

	// 시드(VoronoiPoints)들의 보로노이 셀을 해석적으로 계산해 영역별 볼록 폴리곤을 반환한다(반평면 교집합).
	// InsetCells: 경계 밴드 반영. 각 이등분 반평면을 시드 안쪽으로 시프트해 폴리곤을 셀 단위로 축소(0=인셋 없음).
	// 인셋은 이등분(영역 간) 평면에만 적용하고 그리드 외곽 변에는 미적용.
	void BuildRegionPolygons(const FMapGrid& Grid, float InsetCells, TArray<FRegionPolygon>& OutPolygons) const;

	// === WFC (dormant: 런타임 루프 미사용, 코드 보존) ===
	// 구현은 MapGeneratorSubsystem_WFC.cpp. 관련 데이터 테이블(MapAssetTable/MapWFCTable)은 지연 로드된다.

	// WFC 타일 카탈로그 조회. 첫 호출 시 MapAssetTable을 지연 로드
	const FMapAssetData* FindAsset(FName AssetID) const;

	// WFC 파라미터 행 조회. 첫 호출 시 MapWFCTable을 지연 로드 (RowName 기준)
	const FMapWFCData* FindWFCData(FName MapName) const;

	// 높이 그리드 → WFC로 3D 타일 그리드 생성 (오케스트레이터)
	UFUNCTION(BlueprintCallable, Category = "MapGenerator|WFC")
	FMapTileGrid GenerateTileGrid(FName MapName);

	// MapAssetTable 행 + 경계 소켓 2개를 같은 인터닝 맵으로 변환해 카탈로그/경계 마스크 반환
	void BuildTileSet(FName EmptySocket, FName FloorSocket, TArray<FWFCTile>& OutTiles, uint64& OutEmptyMask, uint64& OutFloorMask) const;

	// 양자화 높이에서 각 열의 층 수/Depth/버퍼를 계산해 채움 (WFC 아님)
	void ComputeColumnLevels(const FMapGrid& HeightGrid, const FMapData& Data, FMapTileGrid& OutTiles) const;

	// WFC 코어: 경계 소켓 마스크를 받아 소켓 교집합 제약 전파/붕괴로 TileIndices 채움
	void RunWFC(const FMapGrid& HeightGrid, const TArray<FWFCTile>& Tiles, uint64 EmptyMask, uint64 FloorMask, FMapTileGrid& TileGrid, int32 Seed) const;

protected:
	// FMapData 행 테이블 (맵 생성/노이즈 + 영역 시각화 파라미터). Initialize에서 로드
	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;

	// FMapAssetData 행 테이블 (WFC 타일 카탈로그, dormant). FindAsset 첫 호출 시 지연 로드
	UPROPERTY()
	mutable TObjectPtr<UDataTable> MapAssetTable;

	// FMapWFCData 행 테이블 (WFC 전용 파라미터, dormant). FindWFCData 첫 호출 시 지연 로드
	UPROPERTY()
	mutable TObjectPtr<UDataTable> MapWFCTable;

};
