// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorActor.h"
#include "MapGenerationData.h"
#include "MapGeneratorSubsystem.h"
#include "Engine/World.h"

AMapGenerator::AMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
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

	const bool bSuccess = Subsystem->GenerateLayout(GenerationData, LastResult);
	UE_LOG(LogTemp, Log, TEXT("AMapGenerator::Generate - %s (방 %d개)"),
		bSuccess ? TEXT("성공") : TEXT("실패"), LastResult.Num());
}
