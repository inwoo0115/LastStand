// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapData.generated.h"

class UStaticMesh;   // 소프트 포인터용 전방선언

// 맵 생성(펄린 노이즈) 파라미터. 데이터 테이블 행으로 관리 (FMapAssetData 패턴).
USTRUCT(BlueprintType)
struct FMapData : public FTableRowBase
{
	GENERATED_BODY()

public:
	FMapData();

	// 맵 식별 이름 (데이터 필드; 행 조회 키(RowName)와 별개, FMapAssetData::AssetName 패턴)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FName MapName;

	// 그리드 가로 셀 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Noise")
	int32 GridWidth = 64;

	// 그리드 세로 셀 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Noise")
	int32 GridHeight = 64;

	// 랜덤 시드. 샘플 좌표 오프셋을 결정해 매번 다른 지형을 생성 (같은 시드는 항상 동일 결과)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Noise")
	int32 Seed = 0;

	// 노이즈 확대 비율. 클수록 지형이 완만하고 넓어짐 (0 나눗셈 방지를 위해 사용 시 클램프)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Noise")
	float Scale = 32.0f;

	// 겹칠 노이즈 레이어(옥타브) 수. 많을수록 디테일 증가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Noise", meta = (ClampMin = "1"))
	int32 Octaves = 4;

	// 옥타브마다 진폭 감소율 [0~1]. 낮을수록 상위 옥타브 영향 감소
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Noise", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Persistence = 0.5f;

	// 옥타브마다 주파수 증가율 (>1). 높을수록 촘촘한 디테일
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Noise", meta = (ClampMin = "1.0"))
	float Lacunarity = 2.0f;

	// 정규화된 노이즈에 곱하는 최종 높이 스케일
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Noise")
	float HeightMultiplier = 1.0f;

	// 샘플링 위치 이동. 맵 패닝/영역 선택용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Noise")
	FVector2D Offset = FVector2D::ZeroVector;

	// 후처리: true면 노이즈의 음수 높이를 절댓값(양수)으로 반전 (능선/봉우리 강조)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Processing")
	bool bUseAbsoluteValue = true;

	// 후처리: 영역 경계 존 두께 비율(0~1). 실제 두께(셀) = round(값 × max(GridWidth,GridHeight))
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Processing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BoundaryThicknessRatio = 0.02f;

	// 후처리: 높이 양자화 간격(0~1). 예) 0.2 → -1,-0.8,...,1 중 최근접으로 반올림. 0이면 비활성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Processing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HeightStep = 0.2f;

	// 타일 한 칸의 크기(cm). XY 간격이자 수직 층 간격(큐브 타일)으로 공용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	float CellSize = 100.0f;

	// WFC 결정성 시드 (붕괴 선택에 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	int32 WFCSeed = 0;

	// WFC 경계 소켓: 그리드 가장자리/공중 면이 호환돼야 하는 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FName EmptySocket = TEXT("Empty");

	// WFC 바닥 소켓: z=0 셀의 -Z 면이 호환돼야 하는 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FName FloorSocket = TEXT("Floor");

	// [임시/디버그] 인스턴싱할 메쉬 (기본: 엔진 큐브). 그리드 시각화 전용, 추후 제거 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
	TSoftObjectPtr<UStaticMesh> DebugMesh;
};
