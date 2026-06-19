// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LSUIEventSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInput);
/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSUIEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	FOnInput InventoryInput;
};
