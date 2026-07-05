// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equipment/LSEquipmentBase.h"
#include "DataTable/LSWeaponData.h"
#include "Components/TimelineComponent.h"
#include "LSWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSWeaponBase : public ALSEquipmentBase
{
	GENERATED_BODY()

public:
	virtual void LaunchWeapon();

	virtual void ReleaseWeapon();
	
	virtual void Equipped() override;

	virtual void UnEquipped() override;

	virtual void ReloadWeapon();

	virtual void Aim();

	virtual void AimRelease();

	virtual void Tick(float DeltaSeconds) override;
	
	FTransform GetCurrentOwnerCamera();

	float GetCurrentOwnerSpringArmLength();

	const FWeaponData GetWeaponData();

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void InitEquipment() override;

	UPROPERTY(Replicated)
	FWeaponData WeaponData;

	// Aim Timeline
	FTimeline AimTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Timeline)
	TObjectPtr<class UCurveFloat> AimCurveFloat;

	FOnTimelineFloat OnTimelineFloatCallback{};

	UFUNCTION()
	void AimUpdate(float Value);

	void InitAimCurve();

	UPROPERTY()
	TObjectPtr<class USpringArmComponent> CachedSpringArm;
};
