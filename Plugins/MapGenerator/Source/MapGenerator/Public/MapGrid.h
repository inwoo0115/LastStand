// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapGrid.generated.h"

// 펄린 노이즈로 생성한 맵 높이 그리드. 나중에 WFC(Wave Function Collapse) 입력으로 사용.
USTRUCT(BlueprintType)
struct FMapGrid
{
	GENERATED_BODY()

	// 그리드 가로 셀 개수
	UPROPERTY(BlueprintReadOnly, Category = "MapGrid")
	int32 Width = 0;

	// 그리드 세로 셀 개수
	UPROPERTY(BlueprintReadOnly, Category = "MapGrid")
	int32 Height = 0;

	// 각 셀의 높이 값 (row-major, 인덱스 = y * Width + x)
	UPROPERTY(BlueprintReadOnly, Category = "MapGrid")
	TArray<float> HeightValues;

	// 보로노이 시드 포인트(극댓값 대표) 좌표 목록. 인덱스가 곧 영역 번호
	UPROPERTY(BlueprintReadOnly, Category = "MapGrid")
	TArray<FIntPoint> VoronoiPoints;

	// 각 셀이 속한 보로노이 영역 번호(VoronoiPoints 인덱스). 미할당 = -1, 영역 경계 존 = -2. (인덱스 = y * Width + x)
	UPROPERTY(BlueprintReadOnly, Category = "MapGrid")
	TArray<int32> RegionIndices;
};

// 한 영역의 보로노이 셀(볼록 폴리곤). 시드(VoronoiPoints)들의 보로노이를 해석적으로 계산해 얻는다.
// 보로노이 셀은 항상 볼록·단일 연결이므로 영역당 폴리곤은 정확히 1개.
USTRUCT()
struct FRegionPolygon
{
	GENERATED_BODY()

	// 외곽선이 감싸는 영역 번호(= VoronoiPoints 인덱스, >= 0)
	int32 RegionId = -1;

	// 셀 좌표계(float) 볼록 폴리곤 꼭짓점 순서열(CCW, 닫힘)
	TArray<FVector2D> Points;
};
