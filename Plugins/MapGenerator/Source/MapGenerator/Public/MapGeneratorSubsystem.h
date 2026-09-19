// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MapData.h"
#include "MapGeneratorSubsystem.generated.h"

class UDataTable;
class ULevelStreaming;
class UMapGenerationData;
struct FRandomStream;

// 방 기반 던전 생성/스트리밍 서브시스템. 생성 로직은 서버 권위에서만 호출한다.
UCLASS()
class MAPGENERATOR_API UMapGeneratorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// MapData 테이블에서 행 조회 (RowName 기준)
	const FMapData* FindMapData(FName RowName) const;

	// 데이터 에셋 기반 방 배치 생성. (현재 스텁 — 알고리즘 추후 구현)
	// 성공 시 Out에 배치 리스트를 채우고 true 반환.
	bool GenerateLayout(const UMapGenerationData* Params, TArray<FPlacedRoom>& Out) const;

	// 배치 리스트의 각 방 서브레벨을 로컬로 스트리밍 로드 (서버·클라 각자 호출)
	void LoadRoomInstances(const TArray<FPlacedRoom>& Rooms);

	// 로드한 방 인스턴스를 모두 언로드
	void ClearRoomInstances();

protected:
	// 부모 출구 문과 자식 입구 문(Doors[0])이 서로 반대 방향·월드 위치 일치하도록 자식 배치 트랜스폼 계산
	FTransform ComputeChildTransform(const FMapData& Parent, const FTransform& ParentXform,
		int32 ParentExitDoorIdx, const FMapData& Child) const;

	// 후보 방(ChildXform 적용)의 박스볼륨이 이미 배치된 방들의 박스볼륨과 겹치지 않는지 검사
	bool CanPlaceRoom(const FMapData& Child, const FTransform& ChildXform,
		const TArray<FPlacedRoom>& Placed, UDataTable* Table) const;

	// 단일 체인 백트래킹 DFS. 성공 시 OutChain에 자식 방들이 append됨
	bool BuildChain(const FMapData& Current, int32 Count, int32 RoomCount,
		const TArray<FName>& NormalRows, const TArray<FName>& EndRows,
		UDataTable* Table, FRandomStream& Stream, TArray<FPlacedRoom>& OutChain) const;

	// FMapData 행 테이블. Initialize에서 로드
	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;

	// LoadRoomInstances로 생성한 스트리밍 레벨 인스턴스 핸들
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULevelStreaming>> LoadedInstances;
};
