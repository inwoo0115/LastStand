// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LSItemBase.generated.h"

UCLASS()
class LASTSTAND_API ALSItemBase : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ALSItemBase();

	const FName GetItemName();

	// 스폰 시 아이템 정체성 부여 (BeginPlay 전 서버에서 세팅 → 복제)
	void SetItemName(FName InItemName) { ItemName = InItemName; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Item")
	FName ItemName;

};
