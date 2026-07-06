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
	ALSWeaponBase();
	 
	virtual void LaunchWeapon();

	virtual void ReleaseWeapon();
	
	virtual void Equipped() override;

	virtual void UnEquipped() override;

	virtual void ReloadWeapon();

	virtual void Aim();

	virtual void AimRelease();

	virtual void Tick(float DeltaSeconds) override;

	virtual void ActivateEquipment() override;

	virtual void DeActivateEquipment() override;
	
	FTransform GetCurrentOwnerCamera();

	float GetCurrentOwnerSpringArmLength();

	const FWeaponData GetWeaponData();

	void LinkWeaponAnimClassLayer(UClass* LayerClass);

	void UnLinkWeaponAnimClassLayer();


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
		Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> WeaponMesh;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void InitEquipment() override;

	UPROPERTY(Replicated)
	FWeaponData WeaponData;

	UPROPERTY(ReplicatedUsing=OnRepIsActived)
	bool bIsActived = false;

	UFUNCTION()
	void OnRepIsActived();

	// Aim Timeline
	FTimeline AimTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Timeline)
	TObjectPtr<class UCurveFloat> AimCurveFloat;

	FOnTimelineFloat OnTimelineFloatCallback{};

	UFUNCTION()
	void AimUpdate(float Value);

	void InitAimCurve();

	UPROPERTY(Replicated)
	TObjectPtr<class USpringArmComponent> CachedSpringArm;

	UPROPERTY()
	TObjectPtr<UClass> CurrentAnimLayerClass;
};
