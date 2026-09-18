// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MapData.h"
#include "MapGeneratorComponent.generated.h"

// GameState에 부착해 던전 배치를 리플리케이트하는 컴포넌트.
// 서버가 배치를 결정 → PlacedRooms 리플리케이션 → 서버·클라 각자 로컬 스트리밍.
UCLASS(ClassGroup = (MapGenerator), meta = (BlueprintSpawnableComponent))
class MAPGENERATOR_API UMapGeneratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMapGeneratorComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// [서버] 던전을 생성해 배치를 리플리케이트하고 로컬 스트리밍을 시작한다.
	UFUNCTION(BlueprintCallable, Category = "MapGenerator")
	void GenerateDungeon(int32 Seed, int32 TargetRoomCount);

protected:
	// 서버가 결정한 방 배치 리스트. OnRep에서 클라 측 스트리밍 수행
	UPROPERTY(ReplicatedUsing = OnRep_PlacedRooms)
	TArray<FPlacedRoom> PlacedRooms;

	UFUNCTION()
	void OnRep_PlacedRooms();

private:
	// 배치 리스트로 방 서브레벨을 로컬 스트리밍 (서브시스템 위임)
	void StreamPlacedRooms();
};
