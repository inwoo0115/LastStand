// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "MapWFCData.h"
#include "MapWFCVisualizer.generated.h"

// [dormant] WFC 3D 타일 그리드를 타일별 메쉬(HISM)로 시각화하는 액터.
// WFC는 런타임 루프에서 제외된 상태이므로 활성 레벨에는 배치하지 않는다(코드/데이터 보존용).
UCLASS()
class MAPGENERATOR_API AMapWFCVisualizer : public AActor
{
	GENERATED_BODY()

public:
	AMapWFCVisualizer();

protected:
	virtual void BeginPlay() override;

	// 루트 (타일별 HISM들의 부모)
	UPROPERTY(VisibleAnywhere, Category = "MapGenerator")
	TObjectPtr<USceneComponent> Root;

	// 타일 인덱스별 HISM (런타임 생성)
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> TileHISMs;

	// [임시] 사용할 MapData / MapWFCData 행 이름 (RowName)
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	FName MapName;

	// WFC 타일 그리드를 생성해 타일별 HISM 인스턴스로 배치
	void BuildVisualization();
};
