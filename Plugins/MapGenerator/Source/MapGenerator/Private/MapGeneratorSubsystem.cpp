// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorSubsystem.h"
#include "MapAssetData.h"
#include "DataTableSettings.h"

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
			Grid.HeightValues[Y * Width + X] = Normalized * Data->HeightMultiplier;
		}
	}

	return Grid;
}
