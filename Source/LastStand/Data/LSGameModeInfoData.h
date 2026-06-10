// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LSGameModeInfoData.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSGameModeInfoData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Data")
	TArray<TSoftClassPtr<UUserWidget>> GameWidgets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Data")
	TArray<TSoftClassPtr<UUserWidget>> MenuWidgets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Data")
	TArray<TSoftClassPtr<UUserWidget>> ModalWidgets;
};
