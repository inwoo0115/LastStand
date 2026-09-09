// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorSubsystem.h"
#include "DataTableSettings.h"

void UMapGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 영역 생성 루프는 MapDataTable만 사용. WFC 테이블(MapAssetTable/MapWFCTable)은
    // dormant이므로 여기서 로드하지 않고 WFC 경로 첫 호출 시 지연 로드한다.
    const UDataTableSettings* Settings = GetDefault<UDataTableSettings>();
    MapDataTable = Settings->MapDataTable.LoadSynchronous();
}

const FMapData* UMapGeneratorSubsystem::FindMapData(FName MapName) const
{
	if (!MapDataTable)
	{
		return nullptr;
	}

	return MapDataTable->FindRow<FMapData>(MapName, TEXT("UMapGeneratorSubsystem::FindMapData"));
}

FMapGrid UMapGeneratorSubsystem::GenerateHeightGrid(FName MapName)
{
	FMapGrid Grid;

	const FMapData* Data = FindMapData(MapName);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMapGeneratorSubsystem::GenerateHeightGrid - MapData 행 '%s'을(를) 찾을 수 없습니다."), *MapName.ToString());
		return Grid;
	}

	const int32 Width = Data->GridWidth;
	const int32 Height = Data->GridHeight;

	if (Width <= 0 || Height <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMapGeneratorSubsystem::GenerateHeightGrid - 유효하지 않은 그리드 크기 (%d x %d)."), Width, Height);
		return Grid;
	}

	Grid.Width = Width;
	Grid.Height = Height;
	Grid.HeightValues.SetNumUninitialized(Width * Height);

	// 0 나눗셈 방지 클램프
	const float Scale = FMath::Max(Data->Scale, KINDA_SMALL_NUMBER);
	const int32 Octaves = FMath::Max(Data->Octaves, 1);
	const float Persistence = Data->Persistence;
	const float Lacunarity = Data->Lacunarity;

	// 시드 기반 옥타브별 오프셋 생성 (각 옥타브를 서로 다른 노이즈 영역에서 샘플)
	FRandomStream Stream(Data->Seed);
	TArray<FVector2D> OctaveOffsets;
	OctaveOffsets.SetNumUninitialized(Octaves);
	for (int32 Octave = 0; Octave < Octaves; ++Octave)
	{
		OctaveOffsets[Octave] = FVector2D(
			Stream.FRandRange(-100000.0f, 100000.0f) + Data->Offset.X,
			Stream.FRandRange(-100000.0f, 100000.0f) + Data->Offset.Y);
	}

	// 그리드 중심을 원점으로 삼아 스케일 조정 시 중심 기준으로 확대/축소되도록 함
	const float HalfWidth = Width * 0.5f;
	const float HalfHeight = Height * 0.5f;

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			float Amplitude = 1.0f;
			float Frequency = 1.0f;
			float NoiseHeight = 0.0f;
			float AmplitudeSum = 0.0f;

			for (int32 Octave = 0; Octave < Octaves; ++Octave)
			{
				const float SampleX = (X - HalfWidth) / Scale * Frequency + OctaveOffsets[Octave].X;
				const float SampleY = (Y - HalfHeight) / Scale * Frequency + OctaveOffsets[Octave].Y;

				// FMath::PerlinNoise2D는 대략 [-1, 1] 범위 반환
				const float Perlin = FMath::PerlinNoise2D(FVector2D(SampleX, SampleY));

				NoiseHeight += Perlin * Amplitude;
				AmplitudeSum += Amplitude;

				Amplitude *= Persistence;
				Frequency *= Lacunarity;
			}

			// 진폭 합으로 정규화해 [-1, 1] 범위 유지
			const float Normalized = (AmplitudeSum > 0.0f) ? (NoiseHeight / AmplitudeSum) : 0.0f;

			// [후처리] 절댓값 플래그: 음수 높이를 양수로 반전 (정규화 공간, 배율은 파이프라인 끝에서 적용)
			Grid.HeightValues[Y * Width + X] = Data->bUseAbsoluteValue ? FMath::Abs(Normalized) : Normalized;
		}
	}

	// === 후처리 파이프라인 (전부 정규화 [-1,1] 공간에서 수행) ===
	// 극댓값 시드 → JFA 보로노이 영역
	FindLocalMaxima(Grid);
	BuildVoronoiRegions(Grid);

	// 영역 경계 존: 그리드가 클수록 두께가 비례해 넓어짐
	const int32 BoundaryThickness = FMath::RoundToInt(
		FMath::Clamp(Data->BoundaryThicknessRatio, 0.0f, 1.0f) * FMath::Max(Width, Height));
	MarkRegionBoundaries(Grid, BoundaryThickness);

	// 높이 양자화 (정규화 값 기준)
	QuantizeHeights(Grid, Data->HeightStep);

	// 정규화 공간 후처리 완료 → 최종 높이 배율 적용
	for (float& H : Grid.HeightValues)
	{
		H *= Data->HeightMultiplier;
	}

	// [로직 테스트] 후처리 결과 확인용 로그 (시드 개수 / 영역 배정 셀 / 경계 셀)
	// RegionIndices 특수 값: -1 = 미할당, -2 = 영역 경계 존
	const int32 BoundaryRegion = -2;
	int32 AssignedCells = 0;
	int32 BoundaryCells = 0;
	for (int32 Region : Grid.RegionIndices)
	{
		if (Region == BoundaryRegion)
		{
			++BoundaryCells;
		}
		else if (Region != -1)
		{
			++AssignedCells;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("UMapGeneratorSubsystem::GenerateHeightGrid - 시드 %d개, 영역 셀 %d, 경계 셀 %d, 총 %d."),
		Grid.VoronoiPoints.Num(), AssignedCells, BoundaryCells, Width * Height);

	return Grid;
}

void UMapGeneratorSubsystem::FindLocalMaxima(FMapGrid& Grid) const
{
	const int32 Width = Grid.Width;
	const int32 Height = Grid.Height;
	const int32 Num = Width * Height;
	if (Num <= 0)
	{
		return;
	}

	Grid.VoronoiPoints.Reset();
	Grid.RegionIndices.Init(-1, Num);

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const int32 Index = Y * Width + X;
			const float SelfValue = Grid.HeightValues[Index];

			// 8방향 이웃과 비교: 모든 이웃이 자기값 이하여야 극댓값 (경계는 존재하는 이웃만)
			bool bIsLocalMax = true;
			int32 AdjacentRegion = -1;   // 이미 라벨된 인접 극댓값(플래토)이 있으면 그 영역 번호
			for (int32 DY = -1; DY <= 1 && bIsLocalMax; ++DY)
			{
				for (int32 DX = -1; DX <= 1; ++DX)
				{
					if (DX == 0 && DY == 0)
					{
						continue;
					}

					const int32 NX = X + DX;
					const int32 NY = Y + DY;
					if (NX < 0 || NX >= Width || NY < 0 || NY >= Height)
					{
						continue;
					}

					const int32 NeighborIndex = NY * Width + NX;
					if (Grid.HeightValues[NeighborIndex] > SelfValue)
					{
						bIsLocalMax = false;
						break;
					}

					// 이미 시드로 라벨된 인접 극댓값(플래토)의 영역을 기억
					if (AdjacentRegion == -1 && Grid.RegionIndices[NeighborIndex] != -1)
					{
						AdjacentRegion = Grid.RegionIndices[NeighborIndex];
					}
				}
			}

			if (!bIsLocalMax)
			{
				continue;
			}

			if (AdjacentRegion != -1)
			{
				// 먼저 지정된 인접 극댓값과 같은 영역으로 병합 (새 포인트에서 제외)
				Grid.RegionIndices[Index] = AdjacentRegion;
			}
			else
			{
				// 새 보로노이 시드 포인트 등록 (인덱스가 곧 영역 번호)
				Grid.RegionIndices[Index] = Grid.VoronoiPoints.Num();
				Grid.VoronoiPoints.Add(FIntPoint(X, Y));
			}
		}
	}
}

void UMapGeneratorSubsystem::BuildVoronoiRegions(FMapGrid& Grid) const
{
	const int32 Width = Grid.Width;
	const int32 Height = Grid.Height;
	const int32 Num = Width * Height;
	if (Num <= 0 || Grid.VoronoiPoints.Num() == 0)
	{
		return;
	}

	const TArray<FIntPoint>& Points = Grid.VoronoiPoints;

	// 각 셀의 현재 최근접 시드 인덱스 (-1 = 미할당). 시드 포인트 위치를 자기 자신으로 초기화
	TArray<int32> Nearest;
	Nearest.Init(-1, Num);
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		Nearest[Points[i].Y * Width + Points[i].X] = i;
	}

	// 셀(x,y)에서 후보 시드까지 제곱 유클리드 거리
	auto DistSq = [&Points](int32 X, int32 Y, int32 Seed) -> int64
	{
		const int64 DX = Points[Seed].X - X;
		const int64 DY = Points[Seed].Y - Y;
		return DX * DX + DY * DY;
	};

	// JFA: Step을 절반씩 줄이며 8방향 점프 이웃의 시드를 비교해 더 가까운 것으로 갱신
	TArray<int32> ReadBuffer;
	int32 Step = FMath::RoundUpToPowerOfTwo(FMath::Max(Width, Height)) / 2;
	for (; Step >= 1; Step /= 2)
	{
		// 한 패스 내 읽기 버퍼를 고정
		ReadBuffer = Nearest;

		for (int32 Y = 0; Y < Height; ++Y)
		{
			for (int32 X = 0; X < Width; ++X)
			{
				const int32 Index = Y * Width + X;
				int32 Best = ReadBuffer[Index];
				int64 BestDist = (Best != -1) ? DistSq(X, Y, Best) : MAX_int64;

				for (int32 DY = -1; DY <= 1; ++DY)
				{
					for (int32 DX = -1; DX <= 1; ++DX)
					{
						if (DX == 0 && DY == 0)
						{
							continue;
						}

						const int32 NX = X + DX * Step;
						const int32 NY = Y + DY * Step;
						if (NX < 0 || NX >= Width || NY < 0 || NY >= Height)
						{
							continue;
						}

						const int32 NeighborSeed = ReadBuffer[NY * Width + NX];
						if (NeighborSeed == -1)
						{
							continue;
						}

						const int64 D = DistSq(X, Y, NeighborSeed);
						if (D < BestDist)
						{
							BestDist = D;
							Best = NeighborSeed;
						}
					}
				}

				Nearest[Index] = Best;
			}
		}
	}

	Grid.RegionIndices = MoveTemp(Nearest);
}

void UMapGeneratorSubsystem::MarkRegionBoundaries(FMapGrid& Grid, int32 Thickness) const
{
	const int32 Width = Grid.Width;
	const int32 Height = Grid.Height;
	const int32 Num = Width * Height;
	if (Num <= 0 || Grid.VoronoiPoints.Num() == 0)
	{
		return;
	}

	// 4방향 오프셋
	const int32 DX4[4] = { 1, -1, 0, 0 };
	const int32 DY4[4] = { 0, 0, 1, -1 };

	// (a) 초기 경계 탐지: 4방향 이웃 중 영역이 다른 셀이 있으면 경계. BFS 거리(0=경계)로 초기화
	TArray<int32> Dist;
	Dist.Init(-1, Num);
	TArray<int32> Frontier;
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const int32 Index = Y * Width + X;
			const int32 Region = Grid.RegionIndices[Index];

			bool bIsBoundary = false;
			for (int32 D = 0; D < 4; ++D)
			{
				const int32 NX = X + DX4[D];
				const int32 NY = Y + DY4[D];
				if (NX < 0 || NX >= Width || NY < 0 || NY >= Height)
				{
					continue;
				}
				if (Grid.RegionIndices[NY * Width + NX] != Region)
				{
					bIsBoundary = true;
					break;
				}
			}

			if (bIsBoundary)
			{
				Dist[Index] = 0;
				Frontier.Add(Index);
			}
		}
	}

	// (b) 확장: 거리제한 멀티소스 BFS로 dist<=Thickness까지 경계로 편입
	int32 Head = 0;
	while (Head < Frontier.Num())
	{
		const int32 Index = Frontier[Head++];
		const int32 CurDist = Dist[Index];
		if (CurDist >= Thickness)
		{
			continue;
		}

		const int32 X = Index % Width;
		const int32 Y = Index / Width;
		for (int32 D = 0; D < 4; ++D)
		{
			const int32 NX = X + DX4[D];
			const int32 NY = Y + DY4[D];
			if (NX < 0 || NX >= Width || NY < 0 || NY >= Height)
			{
				continue;
			}
			const int32 NIndex = NY * Width + NX;
			if (Dist[NIndex] == -1)
			{
				Dist[NIndex] = CurDist + 1;
				Frontier.Add(NIndex);
			}
		}
	}

	// (c) 적용: 경계 셀은 높이 0, 영역 번호 -2
	const int32 BoundaryRegion = -2;
	for (int32 Index = 0; Index < Num; ++Index)
	{
		if (Dist[Index] != -1)
		{
			Grid.HeightValues[Index] = 0.0f;
			Grid.RegionIndices[Index] = BoundaryRegion;
		}
	}
}

void UMapGeneratorSubsystem::QuantizeHeights(FMapGrid& Grid, float Step) const
{
	if (Step <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	for (float& H : Grid.HeightValues)
	{
		// Step 간격 최근접 배수로 스냅 후 [-1,1] 클램프
		H = FMath::Clamp(FMath::GridSnap(H, Step), -1.0f, 1.0f);
	}
}

namespace
{
	// 볼록 폴리곤을 반평면으로 클리핑(Sutherland–Hodgman). 유지 조건: dot(P - Plane, N) <= 0 (N은 단위 벡터).
	// 결과 꼭짓점을 OutPoly에 채운다(입력/출력 버퍼는 서로 달라야 함).
	void ClipConvexByHalfPlane(const TArray<FVector2D>& InPoly, const FVector2D& Plane, const FVector2D& N, TArray<FVector2D>& OutPoly)
	{
		OutPoly.Reset();
		const int32 Count = InPoly.Num();
		if (Count == 0)
		{
			return;
		}

		for (int32 i = 0; i < Count; ++i)
		{
			const FVector2D& A = InPoly[i];
			const FVector2D& B = InPoly[(i + 1) % Count];
			const double DA = FVector2D::DotProduct(A - Plane, N);
			const double DB = FVector2D::DotProduct(B - Plane, N);
			const bool bAInside = (DA <= 0.0);
			const bool bBInside = (DB <= 0.0);

			if (bAInside)
			{
				OutPoly.Add(A);
			}
			// 변이 평면을 가로지르면 교차점 삽입
			if (bAInside != bBInside)
			{
				const double Denom = DA - DB;
				const double T = (FMath::Abs(Denom) > SMALL_NUMBER) ? (DA / Denom) : 0.0;
				OutPoly.Add(A + (B - A) * T);
			}
		}
	}
}

void UMapGeneratorSubsystem::BuildRegionPolygons(const FMapGrid& Grid, float InsetCells, TArray<FRegionPolygon>& OutPolygons) const
{
	OutPolygons.Reset();

	const TArray<FIntPoint>& Seeds = Grid.VoronoiPoints;
	const int32 SeedCount = Seeds.Num();
	if (SeedCount == 0 || Grid.Width <= 0 || Grid.Height <= 0)
	{
		return;
	}

	// 바운딩 박스(셀 좌표): 셀 인스턴스가 정수 좌표 중심에 ±0.5 셀을 차지하는 visualizer 배치와 정합
	const double MinX = -0.5;
	const double MinY = -0.5;
	const double MaxX = Grid.Width - 0.5;
	const double MaxY = Grid.Height - 0.5;

	// 핑퐁 버퍼 재사용
	TArray<FVector2D> PolyA;
	TArray<FVector2D> PolyB;

	for (int32 i = 0; i < SeedCount; ++i)
	{
		const FVector2D Pi(Seeds[i].X, Seeds[i].Y);

		// 박스 사각형(CCW)으로 시작
		PolyA.Reset();
		PolyA.Add(FVector2D(MinX, MinY));
		PolyA.Add(FVector2D(MaxX, MinY));
		PolyA.Add(FVector2D(MaxX, MaxY));
		PolyA.Add(FVector2D(MinX, MaxY));

		TArray<FVector2D>* Cur = &PolyA;
		TArray<FVector2D>* Next = &PolyB;

		// 다른 모든 시드와의 수직이등분 반평면으로 클리핑(+ 경계 밴드 적용)
		for (int32 j = 0; j < SeedCount && Cur->Num() >= 3; ++j)
		{
			if (j == i)
			{
				continue;
			}

			const FVector2D Pj(Seeds[j].X, Seeds[j].Y);
			FVector2D N = Pj - Pi;
			if (!N.Normalize())   // 시드가 겹치면(정상적으로 없음) 스킵
			{
				continue;
			}

			// 이등분선 중점을 시드 i 쪽으로 InsetCells만큼 이동 → 영역 i를 밴드 두께만큼 축소
			const FVector2D Mid = (Pi + Pj) * 0.5;
			const FVector2D Plane = Mid - N * InsetCells;

			ClipConvexByHalfPlane(*Cur, Plane, N, *Next);
			Swap(Cur, Next);
		}

		if (Cur->Num() >= 3)
		{
			FRegionPolygon& Out = OutPolygons.AddDefaulted_GetRef();
			Out.RegionId = i;
			Out.Points = *Cur;   // 볼록 폴리곤 꼭짓점(CCW)
		}
	}
}
