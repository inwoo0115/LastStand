// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/LSGameState.h"
#include "GameState/Components/LSGameModeInfoComponent.h"
#include "MapGeneratorComponent.h"


ALSGameState::ALSGameState()
{
	InfoComp = CreateDefaultSubobject<ULSGameModeInfoComponent>(TEXT("InfoComp"));
	MapGenComp = CreateDefaultSubobject<UMapGeneratorComponent>(TEXT("MapGenComp"));
}

void ALSGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InfoComp->RemoveWidgetsFromLayer();

	Super::EndPlay(EndPlayReason);
}

ULSCharacterControlData* ALSGameState::GetControlData() const
{
	return InfoComp ? InfoComp->GetControlData() : nullptr;
}
