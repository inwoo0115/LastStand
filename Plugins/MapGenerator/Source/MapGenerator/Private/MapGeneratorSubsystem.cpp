// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorSubsystem.h"
#include "MapAssetData.h"
#include "DataTableSettings.h"

// RegionIndices 특수 값: -1 = 미할당, -2 = 영역 경계 존
static constexpr int32 BoundaryRegion = -2;

void UMapGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UDataTableSettings* Settings = GetDefault<UDataTableSettings>();
    MapAssetTable = Settings->MapAssetTable.LoadSynchronous();
    MapDataTable = Settings->MapDataTable.LoadSynchronous();
}

const FMapAssetData* UMapGeneratorSubsystem::FindAsset(FName AssetID) const
{
	if (!MapAssetTable)
	{
		return nullptr;
	}

	return MapAssetTable->FindRow<FMapAssetData>(AssetID, TEXT("UMapGeneratorSubsystem::FindAsset"));
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
		// 한 패스 내 오염 방지를 위해 읽기 버퍼를 고정(핑퐁)
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

FMapTileGrid UMapGeneratorSubsystem::GenerateTileGrid(FName MapName)
{
	FMapTileGrid Tiles;

	const FMapData* Data = FindMapData(MapName);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMapGeneratorSubsystem::GenerateTileGrid - MapData 행 '%s'을(를) 찾을 수 없습니다."), *MapName.ToString());
		return Tiles;
	}

	// 입력 높이 그리드 생성
	const FMapGrid Height = GenerateHeightGrid(MapName);

	// 타일 카탈로그(int 인덱스) 구성 + 경계 소켓 마스크 확보
	TArray<FWFCTile> TileSet;
	uint64 EmptyMask = 0;
	uint64 FloorMask = 0;
	BuildTileSet(Data->EmptySocket, Data->FloorSocket, TileSet, EmptyMask, FloorMask);

	// 각 열의 층 수/Depth/버퍼 계산
	ComputeColumnLevels(Height, *Data, Tiles);

	// 카탈로그 인덱스 → RowName 매핑을 결과에 복사(소비 측 메쉬 조회용)
	Tiles.TileNames.Reset(TileSet.Num());
	for (const FWFCTile& Tile : TileSet)
	{
		Tiles.TileNames.Add(Tile.RowName);
	}

	// WFC 코어: 소켓 제약 전파/붕괴로 TileIndices 채움
	RunWFC(Height, TileSet, EmptyMask, FloorMask, Tiles, Data->WFCSeed);

	// [로직 테스트] 데이터 흐름 확인 로그
	int32 TotalCells = 0;
	for (int32 Levels : Tiles.ColumnLevels)
	{
		TotalCells += Levels;
	}
	UE_LOG(LogTemp, Log, TEXT("UMapGeneratorSubsystem::GenerateTileGrid - 타일 종류 %d개, Depth %d, 총 타일 칸 %d (%d x %d)."),
		Tiles.TileNames.Num(), Tiles.Depth, TotalCells, Tiles.Width, Tiles.Height);

	return Tiles;
}

void UMapGeneratorSubsystem::BuildTileSet(FName EmptySocket, FName FloorSocket, TArray<FWFCTile>& OutTiles, uint64& OutEmptyMask, uint64& OutFloorMask) const
{
	OutTiles.Reset();
	OutEmptyMask = 0;
	OutFloorMask = 0;
	if (!MapAssetTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMapGeneratorSubsystem::BuildTileSet - MapAssetTable이 로드되지 않았습니다."));
		return;
	}

	// 소켓 FName → 비트 인덱스 인터닝 (0부터). 마스크 교집합으로 호환 판정.
	TMap<FName, int32> SocketBits;
	bool bOverflow = false;
	auto BitFor = [&SocketBits, &bOverflow](FName Socket) -> int32
	{
		if (const int32* Found = SocketBits.Find(Socket))
		{
			return *Found;
		}
		const int32 NewBit = SocketBits.Num();
		if (NewBit >= 64)
		{
			bOverflow = true;
			return -1;   // uint64로 표현 불가
		}
		SocketBits.Add(Socket, NewBit);
		return NewBit;
	};
	auto MaskForList = [&BitFor](const TArray<FName>& Names) -> uint64
	{
		uint64 Mask = 0;
		for (const FName& Name : Names)
		{
			if (Name.IsNone()) { continue; }   // 빈 태그는 비트 없음
			const int32 Bit = BitFor(Name);
			if (Bit >= 0) { Mask |= (1ull << Bit); }
		}
		return Mask;
	};

	// 경계 소켓도 타일 소켓과 동일 비트 공간에서 인터닝 (일치 보장). 단일 비트 마스크.
	const int32 EmptyBit = BitFor(EmptySocket);
	const int32 FloorBit = BitFor(FloorSocket);
	OutEmptyMask = (EmptyBit >= 0) ? (1ull << EmptyBit) : 0;
	OutFloorMask = (FloorBit >= 0) ? (1ull << FloorBit) : 0;

	const TArray<FName> RowNames = MapAssetTable->GetRowNames();
	OutTiles.Reserve(RowNames.Num());
	for (const FName& RowName : RowNames)
	{
		const FMapAssetData* Row = FindAsset(RowName);
		if (!Row)
		{
			continue;
		}

		FWFCTile Tile;
		Tile.RowName = RowName;
		Tile.Weight = Row->Weight;
		Tile.SocketMasks[0] = MaskForList(Row->SocketPosX);
		Tile.SocketMasks[1] = MaskForList(Row->SocketNegX);
		Tile.SocketMasks[2] = MaskForList(Row->SocketPosY);
		Tile.SocketMasks[3] = MaskForList(Row->SocketNegY);
		Tile.SocketMasks[4] = MaskForList(Row->SocketPosZ);
		Tile.SocketMasks[5] = MaskForList(Row->SocketNegZ);
		OutTiles.Add(Tile);
	}

	if (bOverflow)
	{
		UE_LOG(LogTemp, Error, TEXT("UMapGeneratorSubsystem::BuildTileSet - 서로 다른 소켓 종류가 64개를 초과했습니다. 일부 소켓이 무시됩니다(uint64 상한)."));
	}
}

void UMapGeneratorSubsystem::ComputeColumnLevels(const FMapGrid& HeightGrid, const FMapData& Data, FMapTileGrid& OutTiles) const
{
	const int32 Width = HeightGrid.Width;
	const int32 Height = HeightGrid.Height;
	OutTiles.Width = Width;
	OutTiles.Height = Height;
	OutTiles.Depth = 0;
	OutTiles.ColumnLevels.Reset();
	OutTiles.TileIndices.Reset();

	const int32 CellCount = Width * Height;
	if (CellCount <= 0)
	{
		return;
	}

	// 저장 높이는 정규화 값에 HeightMultiplier가 곱해진 상태 → 유효 스텝도 배율만큼 확대
	const float EffStep = Data.HeightStep * Data.HeightMultiplier;

	OutTiles.ColumnLevels.SetNumUninitialized(CellCount);
	for (int32 Index = 0; Index < CellCount; ++Index)
	{
		const float H = HeightGrid.HeightValues[Index];
		// 양자화 높이 → 층 인덱스. 열의 층 수 = level + 1 (level 0 포함)
		const int32 Level = (EffStep > KINDA_SMALL_NUMBER) ? FMath::Max(0, FMath::RoundToInt(H / EffStep)) : 0;
		const int32 Levels = Level + 1;
		OutTiles.ColumnLevels[Index] = Levels;
		OutTiles.Depth = FMath::Max(OutTiles.Depth, Levels);
	}

	// 3D 타일 버퍼를 -1(빈칸/미붕괴)로 초기화
	OutTiles.TileIndices.Init(-1, CellCount * OutTiles.Depth);
}

namespace
{
	// 워드 비트셋 헬퍼 (TArray<uint32> 기반). N개 타일 가능성을 비트로 표현.
	namespace WFCBits
	{
		FORCEINLINE int32 NumWords(int32 N) { return (N + 31) >> 5; }

		// 하위 N비트를 1로 세팅 (잉여 비트는 0)
		FORCEINLINE void SetRange(uint32* Words, int32 NumWords, int32 N)
		{
			for (int32 i = 0; i < NumWords; ++i) { Words[i] = 0xFFFFFFFFu; }
			const int32 Rem = N & 31;
			if (Rem != 0) { Words[NumWords - 1] = (1u << Rem) - 1u; }
		}

		FORCEINLINE void SetBit(uint32* Words, int32 Bit) { Words[Bit >> 5] |= (1u << (Bit & 31)); }

		FORCEINLINE int32 Count(const uint32* Words, int32 NumWords)
		{
			int32 C = 0;
			for (int32 i = 0; i < NumWords; ++i) { C += FMath::CountBits((uint64)Words[i]); }
			return C;
		}

		FORCEINLINE bool IsEmpty(const uint32* Words, int32 NumWords)
		{
			for (int32 i = 0; i < NumWords; ++i) { if (Words[i] != 0) { return false; } }
			return true;
		}

		FORCEINLINE void Or(uint32* Dst, const uint32* Src, int32 NumWords)
		{
			for (int32 i = 0; i < NumWords; ++i) { Dst[i] |= Src[i]; }
		}

		// Dst &= Src, 바뀌면 true
		FORCEINLINE bool AndChanged(uint32* Dst, const uint32* Src, int32 NumWords)
		{
			bool bChanged = false;
			for (int32 i = 0; i < NumWords; ++i)
			{
				const uint32 New = Dst[i] & Src[i];
				if (New != Dst[i]) { bChanged = true; Dst[i] = New; }
			}
			return bChanged;
		}

		// 첫 세트 비트 인덱스(없으면 -1)
		FORCEINLINE int32 FirstSetBit(const uint32* Words, int32 NumWords)
		{
			for (int32 w = 0; w < NumWords; ++w)
			{
				if (Words[w] != 0) { return (w << 5) + (int32)FMath::CountTrailingZeros(Words[w]); }
			}
			return -1;
		}
	}
}

void UMapGeneratorSubsystem::RunWFC(const FMapGrid& HeightGrid, const TArray<FWFCTile>& Tiles, uint64 EmptyMask, uint64 FloorMask, FMapTileGrid& TileGrid, int32 Seed) const
{
	using namespace WFCBits;

	const int32 W = TileGrid.Width;
	const int32 H = TileGrid.Height;
	const int32 D = TileGrid.Depth;
	const int32 N = Tiles.Num();
	if (W <= 0 || H <= 0 || D <= 0 || N <= 0)
	{
		if (N <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("UMapGeneratorSubsystem::RunWFC - 타일 카탈로그가 비어 있습니다."));
		}
		return;
	}

	const int32 NW = NumWords(N);
	const int32 Area = W * H;
	const int32 Vol = Area * D;

	// 방향 테이블: 0:+X,1:-X,2:+Y,3:-Y,4:+Z,5:-Z
	const int32 DX[6] = { 1, -1, 0, 0, 0, 0 };
	const int32 DY[6] = { 0, 0, 1, -1, 0, 0 };
	const int32 DZ[6] = { 0, 0, 0, 0, 1, -1 };
	const int32 Opposite[6] = { 1, 0, 3, 2, 5, 4 };

	// 호환 테이블 Compatible[(d*N + t)*NW ..] = { s | (Tiles[t].SocketMasks[d] & Tiles[s].SocketMasks[opp(d)]) != 0 }
	TArray<uint32> Compatible;
	Compatible.Init(0, 6 * N * NW);
	for (int32 d = 0; d < 6; ++d)
	{
		const int32 Od = Opposite[d];
		for (int32 t = 0; t < N; ++t)
		{
			uint32* Row = &Compatible[(d * N + t) * NW];
			const uint64 FaceMask = Tiles[t].SocketMasks[d];
			for (int32 s = 0; s < N; ++s)
			{
				if ((Tiles[s].SocketMasks[Od] & FaceMask) != 0) { SetBit(Row, s); }
			}
		}
	}

	auto IsSolid = [&](int32 X, int32 Y, int32 Z) -> bool
	{
		if (X < 0 || X >= W || Y < 0 || Y >= H || Z < 0) { return false; }
		return Z < TileGrid.ColumnLevels[Y * W + X];
	};

	// 솔리드 셀 목록
	TArray<int32> SolidCells;
	SolidCells.Reserve(Vol);
	for (int32 Z = 0; Z < D; ++Z)
	{
		for (int32 Y = 0; Y < H; ++Y)
		{
			for (int32 X = 0; X < W; ++X)
			{
				if (IsSolid(X, Y, Z)) { SolidCells.Add(Z * Area + Y * W + X); }
			}
		}
	}
	if (SolidCells.Num() == 0)
	{
		return;
	}

	// 경계 필터로 BaseWave 초기 가능성 구성 (비솔리드 셀 워드는 0 유지)
	TArray<uint32> BaseWave;
	BaseWave.Init(0, Vol * NW);
	for (const int32 Cell : SolidCells)
	{
		const int32 X = Cell % W;
		const int32 Y = (Cell / W) % H;
		const int32 Z = Cell / Area;
		uint32* P = &BaseWave[Cell * NW];

		for (int32 t = 0; t < N; ++t)
		{
			if (Tiles[t].Weight <= 0.0f) { continue; }   // 가중치 0 이하 후보 제외

			bool bOk = true;
			for (int32 d = 0; d < 6 && bOk; ++d)
			{
				if (!IsSolid(X + DX[d], Y + DY[d], Z + DZ[d]))
				{
					// 비솔리드 면 → 경계: z=0의 -Z면은 Floor, 그 외는 Empty 비트를 면 마스크가 포함해야 함
					const uint64 Required = (d == 5 && Z == 0) ? FloorMask : EmptyMask;
					if ((Tiles[t].SocketMasks[d] & Required) == 0) { bOk = false; }
				}
			}
			if (bOk) { SetBit(P, t); }
		}

		if (IsEmpty(P, NW))
		{
			UE_LOG(LogTemp, Error, TEXT("UMapGeneratorSubsystem::RunWFC - 셀(%d,%d,%d)에 경계 소켓을 만족하는 타일이 없습니다. 타일 소켓 설정을 확인하세요."), X, Y, Z);
			return;
		}
	}

	// 전파 임시 버퍼
	TArray<uint32> Support;
	Support.SetNumUninitialized(NW);

	// 제약 전파: Stack의 셀들에서 6방향 솔리드 이웃 가능성을 좁힘. 모순 시 false, Counts 갱신.
	auto Propagate = [&](TArray<uint32>& Wave, TArray<int32>& Counts, TArray<int32>& Stack) -> bool
	{
		while (Stack.Num() > 0)
		{
			const int32 Cell = Stack.Pop(EAllowShrinking::No);
			const int32 X = Cell % W;
			const int32 Y = (Cell / W) % H;
			const int32 Z = Cell / Area;
			const uint32* P = &Wave[Cell * NW];

			for (int32 d = 0; d < 6; ++d)
			{
				const int32 NX = X + DX[d];
				const int32 NY = Y + DY[d];
				const int32 NZ = Z + DZ[d];
				if (!IsSolid(NX, NY, NZ)) { continue; }
				const int32 NCell = NZ * Area + NY * W + NX;

				// supported = OR_{t in P} Compatible[d][t]
				for (int32 i = 0; i < NW; ++i) { Support[i] = 0; }
				for (int32 w = 0; w < NW; ++w)
				{
					uint32 Bits = P[w];
					while (Bits != 0)
					{
						const int32 t = (w << 5) + (int32)FMath::CountTrailingZeros(Bits);
						Or(Support.GetData(), &Compatible[(d * N + t) * NW], NW);
						Bits &= Bits - 1;
					}
				}

				uint32* NP = &Wave[NCell * NW];
				if (AndChanged(NP, Support.GetData(), NW))
				{
					const int32 C = Count(NP, NW);
					Counts[NCell] = C;
					if (C == 0) { return false; }
					Stack.Push(NCell);
				}
			}
		}
		return true;
	};

	// 초기 전파(arc consistency): 모든 솔리드 셀에서 경계 제약을 안쪽으로 전파
	TArray<int32> BaseCounts;
	BaseCounts.Init(0, Vol);
	{
		TArray<int32> Stack;
		Stack.Reserve(SolidCells.Num());
		for (const int32 Cell : SolidCells)
		{
			BaseCounts[Cell] = Count(&BaseWave[Cell * NW], NW);
			Stack.Add(Cell);
		}
		if (!Propagate(BaseWave, BaseCounts, Stack))
		{
			UE_LOG(LogTemp, Error, TEXT("UMapGeneratorSubsystem::RunWFC - 초기 제약 전파에서 모순. 타일 소켓 규칙이 경계와 호환되지 않습니다."));
			return;
		}
	}

	// 붕괴 루프 (모순 시 reseed 재시도)
	const int32 MaxAttempts = 20;
	TArray<uint32> Wave;
	TArray<int32> Counts;
	TArray<int32> Stack;
	bool bSuccess = false;
	int32 UsedAttempts = 0;

	for (int32 Attempt = 0; Attempt < MaxAttempts && !bSuccess; ++Attempt)
	{
		UsedAttempts = Attempt + 1;
		Wave = BaseWave;
		Counts = BaseCounts;
		FRandomStream Stream(Seed + Attempt);
		bool bContradiction = false;

		while (true)
		{
			// Observe: 가능성 > 1인 셀 중 최소 카운트 (동률은 리저버 샘플링)
			int32 Best = -1;
			int32 BestCount = MAX_int32;
			int32 Seen = 0;
			for (const int32 Cell : SolidCells)
			{
				const int32 C = Counts[Cell];
				if (C <= 1) { continue; }
				if (C < BestCount)
				{
					BestCount = C;
					Best = Cell;
					Seen = 1;
				}
				else if (C == BestCount)
				{
					++Seen;
					if (Stream.RandRange(0, Seen - 1) == 0) { Best = Cell; }
				}
			}
			if (Best == -1) { break; }   // 전부 붕괴 완료

			// Collapse: 후보 중 최고 Weight 타일 선택 (동률은 리저버 샘플링 → 시드 결정성)
			uint32* P = &Wave[Best * NW];
			int32 Chosen = -1;
			float BestWeight = -FLT_MAX;
			int32 WeightTies = 0;
			for (int32 w = 0; w < NW; ++w)
			{
				uint32 Bits = P[w];
				while (Bits != 0)
				{
					const int32 t = (w << 5) + (int32)FMath::CountTrailingZeros(Bits);
					const float Wt = Tiles[t].Weight;
					if (Wt > BestWeight)
					{
						BestWeight = Wt;
						Chosen = t;
						WeightTies = 1;
					}
					else if (Wt == BestWeight)
					{
						++WeightTies;
						if (Stream.RandRange(0, WeightTies - 1) == 0) { Chosen = t; }
					}
					Bits &= Bits - 1;
				}
			}
			if (Chosen == -1) { Chosen = FirstSetBit(P, NW); }

			// 단일 타일로 붕괴
			for (int32 w = 0; w < NW; ++w) { P[w] = 0; }
			SetBit(P, Chosen);
			Counts[Best] = 1;

			// 전파
			Stack.Reset();
			Stack.Add(Best);
			if (!Propagate(Wave, Counts, Stack))
			{
				bContradiction = true;
				break;
			}
		}

		if (!bContradiction) { bSuccess = true; }
	}

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMapGeneratorSubsystem::RunWFC - %d회 시도 모두 모순. best-effort로 채웁니다."), MaxAttempts);
	}

	// 결과 기록: 각 솔리드 셀의 (유일/첫) 후보 타일 인덱스. 공기 셀은 -1 유지.
	for (const int32 Cell : SolidCells)
	{
		TileGrid.TileIndices[Cell] = FirstSetBit(&Wave[Cell * NW], NW);
	}

	UE_LOG(LogTemp, Log, TEXT("UMapGeneratorSubsystem::RunWFC - 솔리드 셀 %d개 붕괴 (시도 %d회, 성공 %s)."),
		SolidCells.Num(), UsedAttempts, bSuccess ? TEXT("O") : TEXT("X"));
}
