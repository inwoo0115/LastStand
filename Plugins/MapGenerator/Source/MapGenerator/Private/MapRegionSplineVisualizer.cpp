// Fill out your copyright notice in the Description page of Project Settings.


#include "MapRegionSplineVisualizer.h"
#include "MapGeneratorSubsystem.h"
#include "MapGrid.h"
#include "MapData.h"

AMapRegionSplineVisualizer::AMapRegionSplineVisualizer()
{
	// 저작 전용 액터라 매 프레임 갱신 불필요
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void AMapRegionSplineVisualizer::ClearSplines()
{
	for (TObjectPtr<USplineComponent>& Spline : RegionSplines)
	{
		if (Spline)
		{
			Spline->DestroyComponent();
		}
	}
	RegionSplines.Reset();
}

void AMapRegionSplineVisualizer::BuildSplines()
{
	ClearSplines();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UMapGeneratorSubsystem* Subsystem = World->GetSubsystem<UMapGeneratorSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapRegionSplineVisualizer::BuildSplines - MapGeneratorSubsystem을 찾을 수 없습니다."));
		return;
	}

	// 셀 크기/경계 두께를 읽기 위한 행 조회
	const FMapData* Data = Subsystem->FindMapData(MapName);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapRegionSplineVisualizer::BuildSplines - MapData 행 '%s'을(를) 찾을 수 없습니다."), *MapName.ToString());
		return;
	}

	// 영역 그리드 생성 (MapGridVisualizer와 동일 결정적 재생성 → 외곽선이 렌더 영역과 정렬)
	const FMapGrid Grid = Subsystem->GenerateHeightGrid(MapName);
	if (Grid.Width <= 0 || Grid.Height <= 0)
	{
		return;
	}

	// 인셋 산출: MarkRegionBoundaries의 밴드 두께와 동일하게 계산
	const int32 Thickness = FMath::RoundToInt(
		FMath::Clamp(Data->BoundaryThicknessRatio, 0.0f, 1.0f) * FMath::Max(Grid.Width, Grid.Height));
	const float InsetCells = (bInsetByBoundary ? (float)Thickness : 0.0f) + ExtraInsetCells;

	// 시드들의 보로노이 셀을 해석적으로 계산 (밴드만큼 인셋)
	TArray<FRegionPolygon> Polygons;
	Subsystem->BuildRegionPolygons(Grid, InsetCells, Polygons);
	if (Polygons.Num() == 0)
	{
		return;
	}

	// MinRegionCells 필터용 영역별 셀 수 집계
	TMap<int32, int32> RegionCellCount;
	for (int32 R : Grid.RegionIndices)
	{
		if (R >= 0)
		{
			RegionCellCount.FindOrAdd(R)++;
		}
	}

	const FVector CellSize = Data->CellSize;
	int32 Built = 0;

	for (const FRegionPolygon& Polygon : Polygons)
	{
		// 너무 작은 영역 스킵
		if (const int32* Cells = RegionCellCount.Find(Polygon.RegionId))
		{
			if (*Cells < MinRegionCells)
			{
				continue;
			}
		}

		if (Polygon.Points.Num() < 3)
		{
			continue;
		}

		// 스플라인 컴포넌트 생성 (에디터에 저장/표시되도록 인스턴스 컴포넌트로 등록)
		const FString BaseName = FString::Printf(TEXT("Region_%d"), Polygon.RegionId);
		const FName UniqueName = MakeUniqueObjectName(this, USplineComponent::StaticClass(), *BaseName);

		USplineComponent* Spline = NewObject<USplineComponent>(this, USplineComponent::StaticClass(), UniqueName);
		Spline->SetupAttachment(Root);
		Spline->SetMobility(EComponentMobility::Movable);
		Spline->RegisterComponent();
		AddInstanceComponent(Spline);

		// 폴리곤 꼭짓점(셀 좌표) → 로컬 월드. MapGridVisualizer의 X*CellSize.X, Y*CellSize.Y 규칙과 동일
		Spline->ClearSplinePoints(false);
		for (const FVector2D& P : Polygon.Points)
		{
			const FVector Local(P.X * CellSize.X, P.Y * CellSize.Y, SplineZ);
			Spline->AddSplinePoint(Local, ESplineCoordinateSpace::Local, false);
		}
		for (int32 p = 0; p < Spline->GetNumberOfSplinePoints(); ++p)
		{
			Spline->SetSplinePointType(p, PointType, false);
		}
		Spline->SetClosedLoop(true, false);
		Spline->UpdateSpline();

		RegionSplines.Add(Spline);
		++Built;
	}

	UE_LOG(LogTemp, Log, TEXT("AMapRegionSplineVisualizer::BuildSplines - 스플라인 %d개 생성 (폴리곤 %d개, 인셋 %.1f셀, %d x %d)."),
		Built, Polygons.Num(), InsetCells, Grid.Width, Grid.Height);
}
