// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "MapData.h"
#include "MapGridVisualizer.generated.h"

// [임시/디버그] FMapGrid 높이 데이터를 HISM 큐브로 시각화하는 액터. 레벨에 배치해 확인용.
// 시각화 전용이므로 Tick을 사용하지 않으며, WFC 파이프라인 완성 시 제거/대체될 수 있음.
UCLASS()
class MAPGENERATOR_API AMapGridVisualizer : public AActor
{
	GENERATED_BODY()

public:
	AMapGridVisualizer();

protected:
	virtual void BeginPlay() override;

	// 인스턴싱 대상 HISM (루트 컴포넌트)
	UPROPERTY(VisibleAnywhere, Category = "MapGenerator")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> HISM;

	// [임시] 사용할 MapData 테이블 행 이름 (RowName)
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	FName MapName;

	// 그리드를 생성해 HISM 인스턴스로 배치
	void BuildVisualization();
};
