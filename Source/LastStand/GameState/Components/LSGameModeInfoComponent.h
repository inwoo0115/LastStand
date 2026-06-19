// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "LSGameModeInfoComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTSTAND_API ULSGameModeInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULSGameModeInfoComponent();

	virtual void BeginPlay() override;

	void RemoveWidgetsFromLayer();

protected:
	// RPC
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnInfoDataRep)
	TObjectPtr<class ULSGameModeInfoData> InfoData;

	UFUNCTION()
	void OnInfoDataRep();

	UPROPERTY()
	TArray<TObjectPtr<class UUserWidget>> WidgetArray;

	void PushWidgetsToLayer(class ULSUISubsystem* UISubsystem, const TArray<TSoftClassPtr<UUserWidget>>& Widgets, FGameplayTag LayerTag);
};
