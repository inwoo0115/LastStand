// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorActor.h"
#include "MapGenerationData.h"
#include "MapGeneratorSubsystem.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AMapGenerator::AMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	// 시드를 모든 클라에 전달
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AMapGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMapGenerator, RuntimeSeed);
	DOREPLIFETIME(AMapGenerator, bSeedReady);
}

void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (!GenerationData)
		{
			UE_LOG(LogTemp, Warning, TEXT("AMapGenerator::BeginPlay - GenerationData가 설정되지 않았습니다."));
			return;
		}

		// 서버가 시드 확정 → 복제. 데디 서버도 콜리전/히트 판정/AI용 지오메트리가 필요하므로 직접 생성
		RuntimeSeed = bUseRandomSeed ? FMath::Rand() : GenerationData->Seed;
		bSeedReady = true;
		GenerateRuntime();
	}
	else if (bSeedReady)
	{
		// OnRep가 BeginPlay보다 먼저 도착한 경우
		GenerateRuntime();
	}
}

void AMapGenerator::OnRep_SeedReady()
{
	// RuntimeSeed는 같은 번치로 이미 적용된 상태. BeginPlay 전이면 BeginPlay에서 처리
	if (bSeedReady && HasActorBegunPlay())
	{
		GenerateRuntime();
	}
}

void AMapGenerator::GenerateRuntime()
{
	if (bRuntimeGenerated || !GenerationData)
	{
		return;
	}

	UWorld* World = GetWorld();
	UMapGeneratorSubsystem* Subsystem = World ? World->GetSubsystem<UMapGeneratorSubsystem>() : nullptr;
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapGenerator::GenerateRuntime - MapGeneratorSubsystem을 찾을 수 없습니다."));
		return;
	}

	bRuntimeGenerated = true;
	const bool bSuccess = Subsystem->GenerateAndStream(GenerationData, RuntimeSeed, LastResult);
	UE_LOG(LogTemp, Log, TEXT("AMapGenerator::GenerateRuntime - %s (%s, 시드 %d, 방 %d개)"),
		bSuccess ? TEXT("성공") : TEXT("실패"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"),
		RuntimeSeed, LastResult.Num());
}

void AMapGenerator::Generate()
{
	if (!GenerationData)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapGenerator::Generate - GenerationData가 설정되지 않았습니다."));
		return;
	}

	UWorld* World = GetWorld();
	UMapGeneratorSubsystem* Subsystem = World ? World->GetSubsystem<UMapGeneratorSubsystem>() : nullptr;
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapGenerator::Generate - MapGeneratorSubsystem을 찾을 수 없습니다."));
		return;
	}

	const bool bSuccess = Subsystem->GenerateAndStream(GenerationData, GenerationData->Seed, LastResult);
	UE_LOG(LogTemp, Log, TEXT("AMapGenerator::Generate - %s (방 %d개)"),
		bSuccess ? TEXT("성공") : TEXT("실패"), LastResult.Num());
}

void AMapGenerator::ClearRooms()
{
	UWorld* World = GetWorld();
	UMapGeneratorSubsystem* Subsystem = World ? World->GetSubsystem<UMapGeneratorSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return;
	}

	Subsystem->ClearRoomInstances();
	LastResult.Reset();
}
