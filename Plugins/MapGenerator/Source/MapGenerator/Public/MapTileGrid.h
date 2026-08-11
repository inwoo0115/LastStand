// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapTileGrid.generated.h"

// WFC 내부용 int-인덱스 타일 정의. FName은 최초 변환 시에만, 이후 hot loop는 전부 int.
// (UStruct 아님 — 런타임 solver 전용)
struct FWFCTile
{
	FName RowName;                                  // 카탈로그 인덱스 → 원본 행 (메쉬 조회 등 hot loop 밖)
	float Weight = 1.0f;
	uint64 SocketMasks[6] = { 0, 0, 0, 0, 0, 0 };  // 면별 소켓 비트마스크 (+X,-X,+Y,-Y,+Z,-Z)
};

// WFC로 채운 3D 타일 그리드. TileIndices는 TileNames(=FMapAssetData RowName) 인덱스를 참조.
USTRUCT(BlueprintType)
struct FMapTileGrid
{
	GENERATED_BODY()

	// 가로 셀 개수
	UPROPERTY(BlueprintReadOnly, Category = "MapTileGrid")
	int32 Width = 0;

	// 세로 셀 개수
	UPROPERTY(BlueprintReadOnly, Category = "MapTileGrid")
	int32 Height = 0;

	// 최대 층 수 (모든 열 중 최대 level+1)
	UPROPERTY(BlueprintReadOnly, Category = "MapTileGrid")
	int32 Depth = 0;

	// 각 (x,y) 열의 층 수. 인덱스 = y * Width + x
	UPROPERTY(BlueprintReadOnly, Category = "MapTileGrid")
	TArray<int32> ColumnLevels;

	// 각 3D 셀의 타일 인덱스(카탈로그). -1 = 빈칸/미붕괴. 인덱스 = z * Width * Height + y * Width + x
	UPROPERTY(BlueprintReadOnly, Category = "MapTileGrid")
	TArray<int32> TileIndices;

	// 타일 인덱스 → FMapAssetData RowName 매핑 (결과 소비 측에서 메쉬 조회용)
	UPROPERTY(BlueprintReadOnly, Category = "MapTileGrid")
	TArray<FName> TileNames;
};
