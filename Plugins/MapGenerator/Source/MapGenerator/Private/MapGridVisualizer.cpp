// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGridVisualizer.h"
#include "MapGeneratorSubsystem.h"
#include "MapTileGrid.h"
#include "MapAssetData.h"
#include "Engine/StaticMesh.h"

AMapGridVisualizer::AMapGridVisualizer()
{
	// 시각화 전용 액터라 매 프레임 갱신 불필요
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
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

	// 셀 크기(수직 공용)를 읽기 위한 행 조회
	const FMapData* Data = Subsystem->FindMapData(MapName);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapGridVisualizer::BuildVisualization - MapData 행 '%s'을(를) 찾을 수 없습니다."), *MapName.ToString());
		return;
	}

	// WFC 3D 타일 그리드 생성
	const FMapTileGrid Tiles = Subsystem->GenerateTileGrid(MapName);
	if (Tiles.Width <= 0 || Tiles.Height <= 0 || Tiles.Depth <= 0)
	{
		return;
	}

	const float CellSize = Data->CellSize;
	const int32 W = Tiles.Width;
	const int32 H = Tiles.Height;
	const int32 Area = W * H;

	// 폴백 메쉬 (타일에 메쉬 미지정 시)
	UStaticMesh* FallbackMesh = Data->DebugMesh.LoadSynchronous();
	if (!FallbackMesh)
	{
		FallbackMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}

	// 타일 인덱스별 HISM (지연 생성)
	TileHISMs.Reset();
	TileHISMs.SetNum(Tiles.TileNames.Num());

	int32 InstanceCount = 0;
	for (int32 Z = 0; Z < Tiles.Depth; ++Z)
	{
		for (int32 Y = 0; Y < H; ++Y)
		{
			for (int32 X = 0; X < W; ++X)
			{
				const int32 TileIdx = Tiles.TileIndices[Z * Area + Y * W + X];
				if (TileIdx < 0 || TileIdx >= TileHISMs.Num())
				{
					continue;   // 빈칸/미붕괴
				}

				// 해당 타일 인덱스의 HISM 지연 생성
				UHierarchicalInstancedStaticMeshComponent* Comp = TileHISMs[TileIdx];
				if (!Comp)
				{
					UStaticMesh* Mesh = FallbackMesh;
					if (const FMapAssetData* Asset = Subsystem->FindAsset(Tiles.TileNames[TileIdx]))
					{
						if (UStaticMesh* Loaded = Asset->Mesh.LoadSynchronous())
						{
							Mesh = Loaded;
						}
					}

					Comp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
					Comp->SetupAttachment(Root);
					Comp->SetStaticMesh(Mesh);
					Comp->RegisterComponent();
					TileHISMs[TileIdx] = Comp;
				}

				const FVector Location(X * CellSize, Y * CellSize, Z * CellSize);
				Comp->AddInstance(FTransform(Location));
				++InstanceCount;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AMapGridVisualizer::BuildVisualization - 타일 인스턴스 %d개 배치 (%d x %d x %d)."),
		InstanceCount, W, H, Tiles.Depth);
}
