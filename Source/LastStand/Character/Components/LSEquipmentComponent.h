// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataTable/LSItemData.h"
#include "LSEquipmentComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTSTAND_API ULSEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULSEquipmentComponent();

	void EquipItemFromInventory(FName ItemName);

	void UnEquipItemFromInventory(EEquipmentType EquipType);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// RPC
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 장착 무기
	UPROPERTY(EditAnywhere, Category = Equipment, Meta = (AllowPrivateAccess = "true"))
	TMap<EEquipmentType, TObjectPtr<AActor>> Equipments;

};
