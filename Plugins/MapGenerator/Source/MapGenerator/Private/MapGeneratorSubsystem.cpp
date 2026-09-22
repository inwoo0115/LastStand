// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorSubsystem.h"
#include "DataTableSettings.h"
#include "MapGenerationData.h"
#include "Engine/DataTable.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Math/RandomStream.h"

namespace
{
	// 출구 문 인덱스 목록 (ExitDoors 전체 인덱스 — 입구는 별도 필드라 자동 제외)
	TArray<int32> ExitDoorIndices(const FMapData& Room)
	{
		TArray<int32> Exits;
		for (int32 i = 0; i < Room.ExitDoors.Num(); ++i)
		{
			Exits.Add(i);
		}
		return Exits;
	}

	const FMapData* FindRow(UDataTable* Table, FName Row)
	{
		return Table->FindRow<FMapData>(Row, TEXT("MapGenerator"), false);
	}

	// 문 방향 → yaw(도). North=+X(0), East=+Y(90), South=-X(180), West=-Y(270)
	float YawOfDir(EDoorDirection Dir)
	{
		switch (Dir)
		{
		case EDoorDirection::East:  return 90.f;
		case EDoorDirection::South: return 180.f;
		case EDoorDirection::West:  return 270.f;
		default:                    return 0.f;   // North
		}
	}

	EDoorDirection OppositeDir(EDoorDirection Dir)
	{
		switch (Dir)
		{
		case EDoorDirection::South: return EDoorDirection::North;
		case EDoorDirection::East:  return EDoorDirection::West;
		case EDoorDirection::West:  return EDoorDirection::East;
		default:                    return EDoorDirection::South;   // North → South
		}
	}

	// yaw(도, 90° 배수)를 EDoorDirection으로 스냅
	EDoorDirection DirFromYaw(float Yaw)
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

	// 문 방향에 배치 yaw를 더한 월드 방향
	EDoorDirection RotateDirByYaw(EDoorDirection Dir, float Yaw)
	{
		return DirFromYaw(YawOfDir(Dir) + Yaw);
	}

	// 방의 BoundsBoxes를 배치 트랜스폼으로 옮긴 world AABB 목록 생성
	void BuildWorldBoxes(const FMapData& Data, const FTransform& X, TArray<FBox>& Out)
	{
		Out.Reset();
		const FQuat Rot = X.GetRotation();
		for (const FRoomBox& Box : Data.BoundsBoxes)
		{
			const FVector WCenter = X.TransformPosition(Box.Center);
			const FVector WExtent = Rot.RotateVector(Box.Extent).GetAbs();   // 90° 회전 → X/Y 스왑 자동
			Out.Emplace(WCenter - WExtent, WCenter + WExtent);
		}
	}

	// 두 AABB가 모든 축에서 Eps보다 크게 겹치는가 (접촉·공유 벽면은 충돌 아님)
	bool BoxesOverlap(const FBox& A, const FBox& B, float Eps)
	{
		return (A.Min.X < B.Max.X - Eps) && (A.Max.X > B.Min.X + Eps)
			&& (A.Min.Y < B.Max.Y - Eps) && (A.Max.Y > B.Min.Y + Eps)
			&& (A.Min.Z < B.Max.Z - Eps) && (A.Max.Z > B.Min.Z + Eps);
	}
}

FTransform UMapGeneratorSubsystem::ComputeChildTransform(const FMapData& Parent, const FTransform& ParentXform,
	int32 ParentExitDoorIdx, const FMapData& Child) const
{
	if (!Parent.ExitDoors.IsValidIndex(ParentExitDoorIdx) || !Child.bHasEntrance)
	{
		return FTransform::Identity;   // 방어
	}

	// 부모 출구 문의 월드 위치/방향
	const FRoomDoor& PDoor = Parent.ExitDoors[ParentExitDoorIdx];
	const FVector PWorldPos = ParentXform.TransformPosition(PDoor.Location);
	const EDoorDirection PWorldDir = RotateDirByYaw(PDoor.Direction, ParentXform.Rotator().Yaw);

	// 자식 입구 문이 부모 출구의 반대 방향을 향하도록 자식 yaw 결정 (90° 배수)
	const FRoomDoor& CDoor = Child.EntranceDoor;
	const float ChildYaw = YawOfDir(OppositeDir(PWorldDir)) - YawOfDir(CDoor.Direction);
	const FRotator Rot(0.f, ChildYaw, 0.f);

	// 자식 입구 문의 월드 위치가 부모 출구 문 위치와 일치하도록 평행이동
	const FVector T = PWorldPos - Rot.RotateVector(CDoor.Location);
	return FTransform(Rot.Quaternion(), T);
}

bool UMapGeneratorSubsystem::CanPlaceRoom(const FMapData& Child, const FTransform& ChildXform,
	const TArray<FMapGenRoom>& Placed, UDataTable* Table) const
{
	const float Epsilon = 1.0f;

	TArray<FBox> CandBoxes;
	BuildWorldBoxes(Child, ChildXform, CandBoxes);
	if (CandBoxes.Num() == 0)
	{
		return true;   // 바운즈가 없으면 충돌 검사 불가 → 허용 (Export Room Bounds 필요)
	}

	TArray<FBox> PlacedBoxes;
	for (const FMapGenRoom& P : Placed)
	{
		const FMapData* PData = P.Data ? P.Data : FindRow(Table, P.Row);
		if (!PData)
		{
			continue;
		}

		BuildWorldBoxes(*PData, P.Xform, PlacedBoxes);
		for (const FBox& C : CandBoxes)
		{
			for (const FBox& PB : PlacedBoxes)
			{
				if (BoxesOverlap(C, PB, Epsilon))
				{
					return false;   // 겹침 → 배치 불가
				}
			}
		}
	}

	return true;
}

bool UMapGeneratorSubsystem::BuildChain(const FMapData& Current, int32 Count, int32 RoomCount,
	const TArray<FName>& NormalRows, const TArray<FName>& EndRows,
	UDataTable* Table, FRandomStream& Stream, TArray<FMapGenRoom>& Chain) const
{
	// 진입 시 Current == Chain 마지막 방(방금 배치된 부모)
	const int32 ParentIdx = Chain.Num() - 1;
	const FTransform ParentXform = Chain[ParentIdx].Xform;

	// 부모의 열린 출구 문을 랜덤 순서로 시도 (Fisher–Yates)
	TArray<int32> Exits = Chain[ParentIdx].OpenDoors;
	for (int32 i = Exits.Num() - 1; i > 0; --i)
	{
		Exits.Swap(i, Stream.RandRange(0, i));
	}

	const bool bPlaceEnd = (Count >= RoomCount - 1);   // 마지막 Normal → End 배치 단계

	for (int32 DoorIdx : Exits)
	{
		const TArray<FName>& Pool = bPlaceEnd ? EndRows : NormalRows;

		// 이 문에 대해 모든 후보 방을 랜덤 순서로 검사 (Fisher–Yates)
		TArray<FName> Candidates = Pool;
		for (int32 i = Candidates.Num() - 1; i > 0; --i)
		{
			Candidates.Swap(i, Stream.RandRange(0, i));
		}

		for (const FName& Row : Candidates)
		{
			const FMapData* Data = FindRow(Table, Row);
			if (!Data || !Data->bHasEntrance)   // 입구 문이 없으면 연결 불가
			{
				continue;
			}

			const FTransform Xform = ComputeChildTransform(Current, ParentXform, DoorIdx, *Data);
			if (!CanPlaceRoom(*Data, Xform, Chain, Table))   // 박스볼륨 충돌 시 다음 후보
			{
				continue;
			}

			FMapGenRoom Child;
			Child.Row = Row;
			Child.Data = Data;
			Child.Xform = Xform;
			Child.OpenDoors = ExitDoorIndices(*Data);   // 자식의 여분 출구
			Child.bExcludeFromSideRooms = bPlaceEnd;    // 끝방은 사이드룸 제외

			Chain[ParentIdx].OpenDoors.Remove(DoorIdx); // 부모의 사용 문 소비
			Chain.Add(Child);

			if (bPlaceEnd)
			{
				return true;   // End 배치 완료 → 종료
			}
			if (BuildChain(*Data, Count + 1, RoomCount, NormalRows, EndRows, Table, Stream, Chain))
			{
				return true;   // 다음 방으로 내려감 성공
			}

			// 백트래킹 → 자식 제거 + 부모 문 복원
			Chain.Pop();
			Chain[ParentIdx].OpenDoors.Add(DoorIdx);
		}
		// 이 문의 모든 후보 실패 → 다음 문
	}

	return false;   // 이 방의 모든 출구 실패 → 상위에서 백트래킹
}

void UMapGeneratorSubsystem::GrowSideRooms(TArray<FMapGenRoom>& Rooms, const TArray<FName>& NormalRows,
	UDataTable* Table, FRandomStream& Stream, int32 Depth) const
{
	if (NormalRows.Num() == 0)
	{
		return;
	}

	for (int32 d = 0; d < Depth; ++d)
	{
		const int32 Count = Rooms.Num();   // 이번 패스에서 처리할 방 스냅샷 (새 방은 다음 패스에서)
		for (int32 i = 0; i < Count; ++i)
		{
			if (Rooms[i].bExcludeFromSideRooms)   // 끝방 제외 (시작 입구는 exits에 없어 자동 제외)
			{
				continue;
			}

			const FMapData* ParentData = Rooms[i].Data;
			const FTransform ParentXform = Rooms[i].Xform;
			TArray<int32> Doors = Rooms[i].OpenDoors;   // 복사 (순회 중 변형 대비)

			for (int32 DoorIdx : Doors)
			{
				// 이 문에 대해 Normal 후보를 랜덤 순서로 돌며, 되는 방이 나오면 배치
				TArray<FName> Candidates = NormalRows;
				for (int32 k = Candidates.Num() - 1; k > 0; --k)
				{
					Candidates.Swap(k, Stream.RandRange(0, k));
				}

				for (const FName& Row : Candidates)
				{
					const FMapData* Data = FindRow(Table, Row);
					if (!Data || !Data->bHasEntrance)
					{
						continue;
					}

					const FTransform Xform = ComputeChildTransform(*ParentData, ParentXform, DoorIdx, *Data);
					if (!CanPlaceRoom(*Data, Xform, Rooms, Table))
					{
						continue;   // 안되면 다음 후보
					}

					FMapGenRoom Child;
					Child.Row = Row;
					Child.Data = Data;
					Child.Xform = Xform;
					Child.OpenDoors = ExitDoorIndices(*Data);

					Rooms[i].OpenDoors.Remove(DoorIdx);   // 부모 문 소비
					Rooms.Add(Child);
					break;   // 이 문 배치 완료 → 다음 문
				}
			}
		}
	}
}

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

	if (!Params)
	{
		UE_LOG(LogTemp, Warning, TEXT("GenerateLayout - GenerationData가 null입니다."));
		return false;
	}

	UDataTable* Table = Params->MapDataTable.LoadSynchronous();
	if (!Table)
	{
		UE_LOG(LogTemp, Warning, TEXT("GenerateLayout - MapDataTable을 로드할 수 없습니다."));
		return false;
	}

	// 타입별 RowName 버킷 수집
	TArray<FName> StartRows, EndRows, NormalRows;
	for (const TPair<FName, uint8*>& Row : Table->GetRowMap())
	{
		const FMapData* Data = reinterpret_cast<const FMapData*>(Row.Value);
		if (!Data)
		{
			continue;
		}
		switch (Data->RoomType)
		{
		case ERoomType::Start:  StartRows.Add(Row.Key); break;
		case ERoomType::End:    EndRows.Add(Row.Key); break;
		case ERoomType::Normal: NormalRows.Add(Row.Key); break;
		default: break;
		}
	}

	if (StartRows.Num() == 0 || EndRows.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GenerateLayout - Start/End 방이 테이블에 없습니다."));
		return false;
	}

	const int32 RoomCount = Params->RoomCount;
	if (RoomCount > 2 && NormalRows.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GenerateLayout - 중간 Normal 방이 테이블에 없습니다."));
		return false;
	}

	FRandomStream Stream(Params->Seed);

	// Start 방을 원점에 배치
	const FName StartRow = StartRows[Stream.RandRange(0, StartRows.Num() - 1)];
	const FMapData* StartData = FindRow(Table, StartRow);
	if (!StartData)
	{
		return false;
	}

	TArray<FMapGenRoom> Rooms;
	FMapGenRoom StartRoom;
	StartRoom.Row = StartRow;
	StartRoom.Data = StartData;
	StartRoom.Xform = FTransform::Identity;
	StartRoom.OpenDoors = ExitDoorIndices(*StartData);   // 시작 방 입구(EntranceDoor)는 별도 필드라 자동 제외
	Rooms.Add(StartRoom);

	if (RoomCount > 1)
	{
		if (!BuildChain(*StartData, 1, RoomCount, NormalRows, EndRows, Table, Stream, Rooms))
		{
			UE_LOG(LogTemp, Warning, TEXT("GenerateLayout - 체인 구성 실패(백트래킹 소진)."));
			return false;
		}
	}

	// 체인 완료 후 남은 문에 사이드룸 확장
	GrowSideRooms(Rooms, NormalRows, Table, Stream, Params->SideRoomDepth);

	Out.Reserve(Rooms.Num());
	for (const FMapGenRoom& R : Rooms)
	{
		FPlacedRoom P;
		P.RowName = R.Row;
		P.Transform = R.Xform;
		Out.Add(P);
	}

	UE_LOG(LogTemp, Log, TEXT("GenerateLayout - 성공 (방 %d개)"), Out.Num());
	return true;
}

bool UMapGeneratorSubsystem::GenerateAndStream(const UMapGenerationData* Params, TArray<FPlacedRoom>& Out)
{
	Out.Reset();

	if (!Params)
	{
		UE_LOG(LogTemp, Warning, TEXT("GenerateAndStream - GenerationData가 null입니다."));
		return false;
	}

	UDataTable* Table = Params->MapDataTable.LoadSynchronous();
	if (!Table)
	{
		UE_LOG(LogTemp, Warning, TEXT("GenerateAndStream - MapDataTable을 로드할 수 없습니다."));
		return false;
	}

	if (!GenerateLayout(Params, Out))
	{
		return false;
	}

	ClearRoomInstances();
	LoadRoomInstances(Out, Table);
	return true;
}

void UMapGeneratorSubsystem::LoadRoomInstances(const TArray<FPlacedRoom>& Rooms, UDataTable* Table)
{
	UWorld* World = GetWorld();
	if (!World || !Table)
	{
		return;
	}

	for (const FPlacedRoom& Room : Rooms)
	{
		const FMapData* Data = FindRow(Table, Room.RowName);
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
