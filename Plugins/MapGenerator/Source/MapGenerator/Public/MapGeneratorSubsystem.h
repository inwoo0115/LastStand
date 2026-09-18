// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MapData.h"
#include "MapGeneratorSubsystem.generated.h"

class UDataTable;
class ULevelStreaming;
class UMapGenerationData;

// 방 기반 던전 생성/스트리밍 서브시스템. 생성 로직은 서버 권위에서만 호출한다.
UCLASS()
class MAPGENERATOR_API UMapGeneratorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// MapData 테이블에서 행 조회 (RowName 기준)
	const FMapData* FindMapData(FName RowName) const;

	// 방-문 그래프로 방 배치를 결정한다. (현재 스텁 — 추후 구현)
	// 성공 시 Out에 배치 리스트를 채우고 true 반환.
	bool GenerateLayout(int32 Seed, int32 TargetRoomCount, TArray<FPlacedRoom>& Out) const;

	// 데이터 에셋 기반 생성 (에디터 액터 경로). 현재 스텁 — 알고리즘 추후 구현
	bool GenerateLayout(const UMapGenerationData* Params, TArray<FPlacedRoom>& Out) const;

	// 배치 리스트의 각 방 서브레벨을 로컬로 스트리밍 로드 (서버·클라 각자 호출)
	void LoadRoomInstances(const TArray<FPlacedRoom>& Rooms);

	// 로드한 방 인스턴스를 모두 언로드
	void ClearRoomInstances();

protected:
	// FMapData 행 테이블. Initialize에서 로드
	UPROPERTY()
	TObjectPtr<UDataTable> MapDataTable;

	// LoadRoomInstances로 생성한 스트리밍 레벨 인스턴스 핸들
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULevelStreaming>> LoadedInstances;
};
