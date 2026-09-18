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

#define LOCTEXT_NAMESPACE "FMapGeneratorEditorModule"

namespace
{
	// 한 DataTable의 각 방 레벨에서 ABoxVolume들을 찾아 행 BoundsBoxes로 export
	void ExportRoomBounds(UDataTable* Table)
	{
		if (!Table)
		{
			return;
		}

		// FMapData 행 테이블만 처리
		const UScriptStruct* RowStruct = Table->GetRowStruct();
		if (!RowStruct || !RowStruct->IsChildOf(FMapData::StaticStruct()))
		{
			UE_LOG(LogTemp, Warning, TEXT("ExportRoomBounds - '%s'은(는) FMapData 행 테이블이 아닙니다."), *Table->GetName());
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

		UE_LOG(LogTemp, Log, TEXT("ExportRoomBounds('%s') - 완료: %d개 행 / 박스 %d개, 박스 없는 행 %d개. 테이블 저장 필요."),
			*Table->GetName(), RowsWithBoxes, TotalBoxes, RowsWithoutBoxes);
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
			ExportRoomBounds(Table);
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
		"ExportRoomBounds",
		LOCTEXT("ExportRoomBounds", "Export Room Bounds"),
		LOCTEXT("ExportRoomBoundsTooltip", "이 MapData 테이블의 각 방 레벨에서 ABoxVolume 바운즈를 행에 export합니다."),
		FSlateIcon(),
		Action);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMapGeneratorEditorModule, MapGeneratorEditor)
