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

	// 각 셀이 속한 보로노이 영역 번호(VoronoiPoints 인덱스). 미할당 = -1. (인덱스 = y * Width + x)
	UPROPERTY(BlueprintReadOnly, Category = "MapGrid")
	TArray<int32> RegionIndices;
};
