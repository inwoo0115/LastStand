// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorComponent.h"
#include "MapGeneratorSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

UMapGeneratorComponent::UMapGeneratorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMapGeneratorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMapGeneratorComponent, PlacedRooms);
}

void UMapGeneratorComponent::GenerateDungeon(int32 Seed, int32 TargetRoomCount)
{
	// 서버 권위에서만 생성 (리슨서버/GameMode BP 등에서 호출 전제)
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	UMapGeneratorSubsystem* Subsystem = World ? World->GetSubsystem<UMapGeneratorSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return;
	}

	TArray<FPlacedRoom> Result;
	if (!Subsystem->GenerateLayout(Seed, TargetRoomCount, Result))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMapGeneratorComponent::GenerateDungeon - 배치 생성 실패."));
		return;
	}

	// 배치 확정 → 리플리케이션 (클라는 OnRep에서 스트리밍) + 서버 로컬 스트리밍
	PlacedRooms = MoveTemp(Result);
	StreamPlacedRooms();
}

void UMapGeneratorComponent::OnRep_PlacedRooms()
{
	// 클라 측 스트리밍
	StreamPlacedRooms();
}

void UMapGeneratorComponent::StreamPlacedRooms()
{
	UWorld* World = GetWorld();
	UMapGeneratorSubsystem* Subsystem = World ? World->GetSubsystem<UMapGeneratorSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return;
	}

	Subsystem->ClearRoomInstances();
	Subsystem->LoadRoomInstances(PlacedRooms);
}
