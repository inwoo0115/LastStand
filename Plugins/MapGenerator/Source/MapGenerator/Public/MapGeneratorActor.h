// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapData.h"
#include "MapGeneratorActor.generated.h"

class UMapGenerationData;

// 던전 생성을 에디터에서 구동하는 제너레이터 액터.
// GenerationData(데이터 에셋)를 들고, 에디터 Generate 버튼으로 서브시스템 생성 로직을 호출한다.
UCLASS()
class MAPGENERATOR_API AMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AMapGenerator();

	// [에디터] 서브시스템의 맵 생성 로직 호출 (현재: 계산 + 로그)
	UFUNCTION(CallInEditor, Category = "MapGenerator")
	void Generate();

protected:
	// 생성 파라미터 데이터 에셋 (BP에서 편집)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapGenerator", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMapGenerationData> GenerationData;

	// [디버그] 마지막 생성 결과 (인스펙터 확인용)
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "MapGenerator", meta = (AllowPrivateAccess = "true"))
	TArray<FPlacedRoom> LastResult;
};
