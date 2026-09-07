// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGridVisualizer.h"
#include "MapGeneratorSubsystem.h"
#include "MapGrid.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

// RegionIndices 특수 값: -1 = 미할당, -2 = 영역 경계 존
static constexpr int32 BoundaryRegion = -2;

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

namespace
{
	// 영역 번호 → 구분 가능한 색. 황금비 해시로 인접 번호도 색이 크게 갈리게 함
	FLinearColor RegionColor(int32 RegionIndex)
	{
		if (RegionIndex == BoundaryRegion)
		{
			return FLinearColor(0.02f, 0.02f, 0.02f);   // 경계: 어두운 색
		}
		const float Hue01 = FMath::Frac(RegionIndex * 0.61803398875f);
		return FLinearColor::MakeFromHSV8((uint8)(Hue01 * 255.0f), 200, 255);
	}
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

	// 셀 크기/메쉬/머티리얼을 읽기 위한 행 조회
	const FMapData* Data = Subsystem->FindMapData(MapName);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapGridVisualizer::BuildVisualization - MapData 행 '%s'을(를) 찾을 수 없습니다."), *MapName.ToString());
		return;
	}

	// 영역 그리드 생성 (WFC 미호출 — 높이/영역까지만)
	const FMapGrid Grid = Subsystem->GenerateHeightGrid(MapName);
	if (Grid.Width <= 0 || Grid.Height <= 0)
	{
		return;
	}

	const FVector CellSize = Data->CellSize;
	const int32 W = Grid.Width;
	const int32 H = Grid.Height;

	// 인스턴싱 메쉬 (미지정 시 엔진 큐브)
	UStaticMesh* Mesh = Data->DebugMesh.LoadSynchronous();
	if (!Mesh)
	{
		Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}

	// 영역 색 구분용 베이스 머티리얼 (선택)
	UMaterialInterface* RegionMaterial = Data->RegionMaterial.LoadSynchronous();
	if (!RegionMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapGridVisualizer::BuildVisualization - RegionMaterial 미지정. 영역이 색으로 구분되지 않습니다(메쉬 기본 머티리얼 사용)."));
	}

	RegionHISMs.Reset();

	int32 InstanceCount = 0;
	for (int32 Y = 0; Y < H; ++Y)
	{
		for (int32 X = 0; X < W; ++X)
		{
			const int32 Region = Grid.RegionIndices[Y * W + X];
			if (Region == -1)
			{
				continue;   // 미할당 셀은 스킵
			}

			// 영역 번호별 HISM 지연 생성
			TObjectPtr<UHierarchicalInstancedStaticMeshComponent>& Slot = RegionHISMs.FindOrAdd(Region);
			if (!Slot)
			{
				UHierarchicalInstancedStaticMeshComponent* Comp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
				Comp->SetupAttachment(Root);
				Comp->SetStaticMesh(Mesh);
				Comp->RegisterComponent();

				// 영역 색 적용 (베이스 머티리얼이 있을 때만)
				if (RegionMaterial)
				{
					UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(RegionMaterial, this);
					MID->SetVectorParameterValue(Data->RegionColorParam, RegionColor(Region));
					Comp->SetMaterial(0, MID);
				}

				Slot = Comp;
			}

			// 평면(Z=0) 배치 — 높이 값은 데이터로만 유지하고 시각화에는 반영하지 않음
			const FVector Location(X * CellSize.X, Y * CellSize.Y, 0.0f);
			Slot->AddInstance(FTransform(Location));
			++InstanceCount;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AMapGridVisualizer::BuildVisualization - 영역 %d개, 인스턴스 %d개 배치 (%d x %d)."),
		RegionHISMs.Num(), InstanceCount, W, H);
}
