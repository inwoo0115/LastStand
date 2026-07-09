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

	// InfoData에 설정된 컨트롤 데이터 반환 (없으면 nullptr)
	class ULSCharacterControlData* GetControlData() const;

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

	// 로컬 플레이어의 캐릭터에 InfoData의 ControlData 적용
	void ApplyControlDataToLocalPawn();
};
