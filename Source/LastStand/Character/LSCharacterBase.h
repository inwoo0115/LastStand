// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LSCharacterBase.generated.h"


UENUM()
enum class ECharacterControlType : uint8
{
	Default
};


UCLASS()
class LASTSTAND_API ALSCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALSCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 컨트롤 데이터 설정
	void SetCharacterControlData(ECharacterControlType ControlType);

	UPROPERTY(ReplicatedUsing = OnRep_ControlType)
	ECharacterControlType CurrentCharacterControlType = ECharacterControlType::Default;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ControlType();

	// 컨트롤 매니저
	UPROPERTY(EditAnywhere, Category = CharacterControl, Meta = (AllowPrivateAccess = "true"))
	TMap<ECharacterControlType, class ULSCharacterControlData*> CharacterControlManager;

	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSInventoryComponent> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSEquipmentComponent> Equipments;
};
