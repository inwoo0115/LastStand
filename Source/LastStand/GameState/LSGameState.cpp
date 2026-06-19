// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/LSGameState.h"
#include "GameState/Components/LSGameModeInfoComponent.h"


ALSGameState::ALSGameState()
{
	InfoComp = CreateDefaultSubobject<ULSGameModeInfoComponent>(TEXT("InfoComp"));
}

void ALSGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InfoComp->RemoveWidgetsFromLayer();

	Super::EndPlay(EndPlayReason);
}
