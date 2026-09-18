// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MapGenerationData.generated.h"

class UDataTable;   // 소프트 포인터용 전방선언

// 던전 생성 파라미터를 담는 데이터 에셋. AMapGenerator가 참조해 생성에 사용.
UCLASS(BlueprintType)
class MAPGENERATOR_API UMapGenerationData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 참조할 MapData 테이블 (FMapData 행). 하드 레퍼런스 금지 — 소프트 포인터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	TSoftObjectPtr<UDataTable> MapDataTable;

	// 시작 지점에서 끝 지점까지 배치할 방 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	int32 RoomCount = 0;

	// 생성 시드 (같은 시드 = 같은 던전)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	int32 Seed = 0;
};
