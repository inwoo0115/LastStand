// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LSItemData.h"
#include "LSDataSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	const FItemData* FindItem(FName ItemID) const;

protected: 
	// 데이터 테이블 포인터
	UPROPERTY()
	TObjectPtr<UDataTable> ItemTable;
};
