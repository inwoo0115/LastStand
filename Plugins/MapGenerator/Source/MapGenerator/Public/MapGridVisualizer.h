// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "MapData.h"
#include "MapGridVisualizer.generated.h"

// [디버그] 보로노이 영역 그리드를 영역별 색으로 시각화하는 액터. 레벨에 배치해 확인용.
// 높이 값은 데이터로만 유지되고 시각화는 평면(Z=0)에 영역만 색으로 구분한다.
// 시각화 전용이므로 Tick을 사용하지 않는다.
UCLASS()
class MAPGENERATOR_API AMapGridVisualizer : public AActor
{
	GENERATED_BODY()

public:
	AMapGridVisualizer();

protected:
	virtual void BeginPlay() override;

	// 루트 (영역별 HISM들의 부모)
	UPROPERTY(VisibleAnywhere, Category = "MapGenerator")
	TObjectPtr<USceneComponent> Root;

	// 영역 번호별 HISM (런타임 생성). 키 = RegionIndices 값
	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> RegionHISMs;

	// [임시] 사용할 MapData 테이블 행 이름 (RowName)
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	FName MapName;

	// 영역 그리드를 생성해 영역별 색 HISM 인스턴스로 평면 배치
	void BuildVisualization();
};
