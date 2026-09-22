// Copyright Epic Games, Inc. All Rights Reserved.

#include "MapGeneratorEditor.h"
#include "ToolMenus.h"
#include "ContentBrowserMenuContexts.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "MapData.h"
#include "BoxVolume.h"
#include "DoorVolume.h"

#define LOCTEXT_NAMESPACE "FMapGeneratorEditorModule"

namespace
{
	// 액터 yaw(임의 각)를 4방위 EDoorDirection으로 스냅.
	// North=+X(0), East=+Y(90), South=-X(180), West=-Y(270) — 서브시스템 DirFromYaw와 동일 로직
	EDoorDirection SnapYawToDirection(float Yaw)
	{
		const int32 Q = ((FMath::RoundToInt(Yaw / 90.f) % 4) + 4) % 4;
		switch (Q)
		{
		case 1:  return EDoorDirection::East;
		case 2:  return EDoorDirection::South;
		case 3:  return EDoorDirection::West;
		default: return EDoorDirection::North;
		}
	}

	// 한 DataTable의 각 방 레벨에서 ABoxVolume/ADoorVolume을 찾아
	// 행의 BoundsBoxes / EntranceDoor·ExitDoors로 export
	void ExportRoomData(UDataTable* Table)
	{
		if (!Table)
		{
			return;
		}

		// FMapData 행 테이블만 처리
		const UScriptStruct* RowStruct = Table->GetRowStruct();
		if (!RowStruct || !RowStruct->IsChildOf(FMapData::StaticStruct()))
		{
			UE_LOG(LogTemp, Warning, TEXT("ExportRoomData - '%s'은(는) FMapData 행 테이블이 아닙니다."), *Table->GetName());
			return;
		}

		int32 RowsWithBoxes = 0;
		int32 TotalBoxes = 0;
		int32 RowsWithoutBoxes = 0;
		int32 RowsWithoutEntrance = 0;
		int32 TotalDoors = 0;

		for (const TPair<FName, uint8*>& Row : Table->GetRowMap())
		{
			FMapData* Data = reinterpret_cast<FMapData*>(Row.Value);
			if (!Data)
			{
				continue;
			}

			Data->BoundsBoxes.Reset();
			Data->ExitDoors.Reset();
			Data->EntranceDoor = FRoomDoor();
			Data->bHasEntrance = false;

			UWorld* World = Data->RoomLevel.LoadSynchronous();
			if (!World || !World->PersistentLevel)
			{
				UE_LOG(LogTemp, Warning, TEXT("ExportRoomData - 행 '%s'의 RoomLevel을 로드할 수 없습니다."), *Row.Key.ToString());
				++RowsWithoutBoxes;
				++RowsWithoutEntrance;
				continue;
			}

			// 입구 볼륨은 여러 개면 오류라 별도로 모아 개수 검증
			TArray<FRoomDoor> EntranceDoors;

			for (AActor* Actor : World->PersistentLevel->Actors)
			{
				if (ABoxVolume* BoxVol = Cast<ABoxVolume>(Actor))
				{
					if (!BoxVol->GetBoxComponent())
					{
						continue;
					}

					FRoomBox Box;
					Box.Center = BoxVol->GetActorLocation();
					Box.Extent = BoxVol->GetBoxComponent()->GetScaledBoxExtent();
					Data->BoundsBoxes.Add(Box);
				}
				else if (ADoorVolume* DoorVol = Cast<ADoorVolume>(Actor))
				{
					FRoomDoor Door;
					Door.Location = DoorVol->GetActorLocation();
					Door.Direction = SnapYawToDirection(DoorVol->GetActorRotation().Yaw);

					if (DoorVol->IsEntrance())
					{
						EntranceDoors.Add(Door);
					}
					else
					{
						Data->ExitDoors.Add(Door);
					}
				}
			}

			// 입구 문 검증 — 정확히 1개일 때만 유효
			if (EntranceDoors.Num() == 1)
			{
				Data->EntranceDoor = EntranceDoors[0];
				Data->bHasEntrance = true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("ExportRoomData - 행 '%s' 레벨의 입구(ADoorVolume bIsEntrance) 개수가 %d개입니다. 정확히 1개여야 합니다 — 입구 미설정."),
					*Row.Key.ToString(), EntranceDoors.Num());
				++RowsWithoutEntrance;
			}

			TotalDoors += (Data->bHasEntrance ? 1 : 0) + Data->ExitDoors.Num();

			if (Data->BoundsBoxes.Num() > 0)
			{
				++RowsWithBoxes;
				TotalBoxes += Data->BoundsBoxes.Num();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ExportRoomData - 행 '%s' 레벨에 ABoxVolume이 없습니다."), *Row.Key.ToString());
				++RowsWithoutBoxes;
			}
		}

		Table->Modify();
		Table->MarkPackageDirty();

		UE_LOG(LogTemp, Log, TEXT("ExportRoomData('%s') - 완료: %d개 행 / 박스 %d개 / 문 %d개, 박스 없는 행 %d개, 입구 없는 행 %d개. 테이블 저장 필요."),
			*Table->GetName(), RowsWithBoxes, TotalBoxes, TotalDoors, RowsWithoutBoxes, RowsWithoutEntrance);
	}

	// 우클릭 메뉴 클릭 핸들러
	void OnExportClicked(const FToolMenuContext& MenuContext)
	{
		const UContentBrowserAssetContextMenuContext* Context = MenuContext.FindContext<UContentBrowserAssetContextMenuContext>();
		if (!Context)
		{
			return;
		}

		TArray<UDataTable*> Tables = Context->LoadSelectedObjects<UDataTable>();
		for (UDataTable* Table : Tables)
		{
			ExportRoomData(Table);
		}
	}
}

void FMapGeneratorEditorModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMapGeneratorEditorModule::RegisterMenus));
}

void FMapGeneratorEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FMapGeneratorEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.DataTable");
	if (!Menu)
	{
		return;
	}

	FToolMenuSection& Section = Menu->FindOrAddSection("MapGenerator", LOCTEXT("MapGeneratorSection", "Map Generator"));

	FToolUIAction Action;
	Action.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&OnExportClicked);

	Section.AddMenuEntry(
		"ExportRoomData",
		LOCTEXT("ExportRoomData", "Export Room Data"),
		LOCTEXT("ExportRoomDataTooltip", "이 MapData 테이블의 각 방 레벨에서 ABoxVolume 바운즈와 ADoorVolume 문(입구/출구)을 행에 export합니다."),
		FSlateIcon(),
		Action);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMapGeneratorEditorModule, MapGeneratorEditor)
