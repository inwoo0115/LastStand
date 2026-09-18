// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTableSettings.h"

#if WITH_EDITOR
#include "MapData.h"
#include "BoxVolume.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Components/BoxComponent.h"

void UDataTableSettings::ExportRoomBounds()
{
	UDataTable* Table = MapDataTable.LoadSynchronous();
	if (!Table)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExportRoomBounds - MapDataTable이 설정되지 않았습니다."));
		return;
	}

	int32 RowsWithBoxes = 0;
	int32 TotalBoxes = 0;
	int32 RowsWithoutBoxes = 0;

	for (const TPair<FName, uint8*>& Row : Table->GetRowMap())
	{
		FMapData* Data = reinterpret_cast<FMapData*>(Row.Value);
		if (!Data)
		{
			continue;
		}

		Data->BoundsBoxes.Reset();

		UWorld* World = Data->RoomLevel.LoadSynchronous();
		if (!World || !World->PersistentLevel)
		{
			UE_LOG(LogTemp, Warning, TEXT("ExportRoomBounds - 행 '%s'의 RoomLevel을 로드할 수 없습니다."), *Row.Key.ToString());
			++RowsWithoutBoxes;
			continue;
		}

		for (AActor* Actor : World->PersistentLevel->Actors)
		{
			ABoxVolume* Volume = Cast<ABoxVolume>(Actor);
			if (!Volume || !Volume->GetBoxComponent())
			{
				continue;
			}

			FRoomBox Box;
			Box.Center = Volume->GetActorLocation();
			Box.Extent = Volume->GetBoxComponent()->GetScaledBoxExtent();
			Data->BoundsBoxes.Add(Box);
		}

		if (Data->BoundsBoxes.Num() > 0)
		{
			++RowsWithBoxes;
			TotalBoxes += Data->BoundsBoxes.Num();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ExportRoomBounds - 행 '%s' 레벨에 ABoxVolume이 없습니다."), *Row.Key.ToString());
			++RowsWithoutBoxes;
		}
	}

	Table->Modify();
	Table->MarkPackageDirty();

	UE_LOG(LogTemp, Log, TEXT("ExportRoomBounds - 완료: %d개 행 / 박스 %d개, 박스 없는 행 %d개. 테이블 저장 필요."),
		RowsWithBoxes, TotalBoxes, RowsWithoutBoxes);
}
#endif
