// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "MapRegionSplineVisualizer.generated.h"

// [저작 도구] 보로노이 영역의 외곽선을 닫힌 스플라인으로 생성하는 액터. 에디터에 배치해 버튼으로 생성.
// 스플라인만 만들며 PCG 연결은 사용자가 에디터에서 수동으로(예: UPCGComponent + PCGForest 그래프로 영역 내부 채우기).
// 시각화/저작 전용이라 Tick 미사용.
UCLASS()
class MAPGENERATOR_API AMapRegionSplineVisualizer : public AActor
{
	GENERATED_BODY()

public:
	AMapRegionSplineVisualizer();

	// 영역 외곽선을 닫힌 스플라인으로 (재)생성. 기존 스플라인은 먼저 제거
	UFUNCTION(CallInEditor, Category = "MapGenerator")
	void BuildSplines();

	// 생성된 스플라인 전부 제거
	UFUNCTION(CallInEditor, Category = "MapGenerator")
	void ClearSplines();

protected:
	// 사용할 MapData 테이블 행 이름 (RowName). MapGridVisualizer와 같은 값이어야 외곽선이 렌더 영역과 정렬
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	FName MapName;

	// [플래그] 스플라인 포인트 타입: Linear(기본, PCG 인테리어 샘플링에 충분) / Curve(영역 외곽을 부드럽게)
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	TEnumAsByte<ESplinePointType::Type> PointType = ESplinePointType::Linear;

	// [플래그] 경계 밴드 두께만큼 폴리곤을 안쪽으로 인셋 → 스플라인이 색칠 영역(밴드 제외)에 밀착.
	// false면 인셋 없이 밴드 중앙(이등분선)에 스플라인
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	bool bInsetByBoundary = true;

	// 밴드 자동 인셋에 더할 수동 인셋(셀 단위, 음수 가능). 미세 조정용
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	float ExtraInsetCells = 0.0f;

	// 이 셀 수 미만인 영역은 스킵(자잘한 영역 제외)
	UPROPERTY(EditAnywhere, Category = "MapGenerator", meta = (ClampMin = "0"))
	int32 MinRegionCells = 8;

	// 스플라인 배치 높이(평면 Z). MapGridVisualizer(Z=0)와 겹쳐 검증하려면 0 유지
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	float SplineZ = 0.0f;

	// 루트 (스플라인들의 부모)
	UPROPERTY(VisibleAnywhere, Category = "MapGenerator")
	TObjectPtr<USceneComponent> Root;

	// 생성된 영역 외곽 스플라인들
	UPROPERTY(VisibleAnywhere, Category = "MapGenerator")
	TArray<TObjectPtr<USplineComponent>> RegionSplines;
};
