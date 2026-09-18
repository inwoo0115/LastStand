// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorSubsystem.h"
#include "DataTableSettings.h"
#include "Engine/DataTable.h"
#include "Engine/LevelStreamingDynamic.h"

void UMapGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 세션 시작 시 MapData 테이블 로드 (LSDataSubsystem::Initialize와 동일 관용)
	const UDataTableSettings* Settings = GetDefault<UDataTableSettings>();
	MapDataTable = Settings->MapDataTable.LoadSynchronous();
}

const FMapData* UMapGeneratorSubsystem::FindMapData(FName RowName) const
{
	if (!MapDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMapGeneratorSubsystem::FindMapData - MapDataTable이 로드되지 않았습니다."));
		return nullptr;
	}

	return MapDataTable->FindRow<FMapData>(RowName, TEXT("UMapGeneratorSubsystem::FindMapData"));
}

bool UMapGeneratorSubsystem::GenerateLayout(const UMapGenerationData* Params, TArray<FPlacedRoom>& Out) const
{
	Out.Reset();

	// TODO: 배치 알고리즘 재작성 예정
	return false;
}

void UMapGeneratorSubsystem::LoadRoomInstances(const TArray<FPlacedRoom>& Rooms)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (const FPlacedRoom& Room : Rooms)
	{
		const FMapData* Data = FindMapData(Room.RowName);
		if (!Data || Data->RoomLevel.IsNull())
		{
			UE_LOG(LogTemp, Warning, TEXT("UMapGeneratorSubsystem::LoadRoomInstances - 방 '%s'의 레벨을 찾을 수 없습니다."), *Room.RowName.ToString());
			continue;
		}

		bool bSuccess = false;
		ULevelStreamingDynamic* Instance = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
			World,
			Data->RoomLevel,
			Room.Transform,
			bSuccess);

		if (bSuccess && Instance)
		{
			LoadedInstances.Add(Instance);
		}
	}
}

void UMapGeneratorSubsystem::ClearRoomInstances()
{
	for (ULevelStreaming* Instance : LoadedInstances)
	{
		if (Instance)
		{
			Instance->SetIsRequestingUnloadAndRemoval(true);
		}
	}

	LoadedInstances.Reset();
}
