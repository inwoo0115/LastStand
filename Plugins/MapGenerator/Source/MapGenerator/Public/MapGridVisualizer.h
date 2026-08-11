// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "MapData.h"
#include "MapGridVisualizer.generated.h"

// [임시/디버그] WFC 3D 타일 그리드를 타일별 메쉬(HISM)로 시각화하는 액터. 레벨에 배치해 확인용.
// 시각화 전용이므로 Tick을 사용하지 않으며, WFC 파이프라인 완성 시 제거/대체될 수 있음.
UCLASS()
class MAPGENERATOR_API AMapGridVisualizer : public AActor
{
	GENERATED_BODY()

public:
	AMapGridVisualizer();

protected:
	virtual void BeginPlay() override;

	// 루트 (타일별 HISM들의 부모)
	UPROPERTY(VisibleAnywhere, Category = "MapGenerator")
	TObjectPtr<USceneComponent> Root;

	// 타일 인덱스별 HISM (런타임 생성)
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> TileHISMs;

	// [임시] 사용할 MapData 테이블 행 이름 (RowName)
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	FName MapName;

	// WFC 타일 그리드를 생성해 타일별 HISM 인스턴스로 배치
	void BuildVisualization();
};
