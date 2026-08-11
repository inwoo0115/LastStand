// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGridVisualizer.h"
#include "MapGeneratorSubsystem.h"
#include "MapGrid.h"
#include "Engine/StaticMesh.h"

AMapGridVisualizer::AMapGridVisualizer()
{
	// 시각화 전용 액터라 매 프레임 갱신 불필요
	PrimaryActorTick.bCanEverTick = false;

	HISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISM"));
	RootComponent = HISM;
}

void AMapGridVisualizer::BeginPlay()
{
	Super::BeginPlay();

	BuildVisualization();
}

void AMapGridVisualizer::BuildVisualization()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UMapGeneratorSubsystem* Subsystem = World->GetSubsystem<UMapGeneratorSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapGridVisualizer::BuildVisualization - MapGeneratorSubsystem을 찾을 수 없습니다."));
		return;
	}

	// 디버그 파라미터(셀 간격/높이 배율/메쉬)를 읽기 위한 행 조회
	const FMapData* Data = Subsystem->FindMapData(MapName);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapGridVisualizer::BuildVisualization - MapData 행 '%s'을(를) 찾을 수 없습니다."), *MapName.ToString());
		return;
	}

	// 높이 그리드 생성
	const FMapGrid Grid = Subsystem->GenerateHeightGrid(MapName);

	if (Grid.Width <= 0 || Grid.Height <= 0)
	{
		return;
	}

	// 인스턴싱할 메쉬 로드 (미지정 시 엔진 큐브로 폴백)
	UStaticMesh* Mesh = Data->DebugMesh.LoadSynchronous();
	if (!Mesh)
	{
		Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}
	HISM->SetStaticMesh(Mesh);

	// 각 셀을 XY 격자로 배치하고 양자화 층(level)만큼 Z를 올림 (타일 큐브 기준, 수직도 CellSize)
	const float CellSize = Data->CellSize;
	// 저장 높이는 정규화 값에 HeightMultiplier가 곱해진 상태 → 유효 스텝도 배율만큼 확대
	const float EffStep = Data->HeightStep * Data->HeightMultiplier;

	HISM->ClearInstances();
	for (int32 Y = 0; Y < Grid.Height; ++Y)
	{
		for (int32 X = 0; X < Grid.Width; ++X)
		{
			const float H = Grid.HeightValues[Y * Grid.Width + X];
			const int32 Level = (EffStep > KINDA_SMALL_NUMBER) ? FMath::Max(0, FMath::RoundToInt(H / EffStep)) : 0;
			const FVector Location(X * CellSize, Y * CellSize, Level * CellSize);
			HISM->AddInstance(FTransform(Location));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AMapGridVisualizer::BuildVisualization - %d개 인스턴스 배치 (%d x %d)."),
		Grid.Width * Grid.Height, Grid.Width, Grid.Height);
}
