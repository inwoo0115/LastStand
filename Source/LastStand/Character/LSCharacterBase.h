// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/LSInteractComponentInterface.h"
#include "Interface/LSInventoryComponentInterface.h"
#include "LSCharacterBase.generated.h"


UENUM()
enum class ECharacterControlType : uint8
{
	Default
};


UCLASS()
class LASTSTAND_API ALSCharacterBase : public ACharacter, public ILSInteractComponentInterface, public ILSInventoryComponentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALSCharacterBase();

	virtual ULSInteractionComponent* GetInteractionComponent() override;

	virtual ULSInventoryComponent* GetInventoryComponent() override;

	class USpringArmComponent* GetSpringArmComponent();

	FTransform GetCurrentCameraTransform() const;

	float GetCurrentSpringArmLength() const;

	FRotator GetCurrentControllerRotation() const;

	virtual void Tick(float DeltaSeconds) override;

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

	// Controller Rotation Replication
	UPROPERTY(Replicated)
	FRotator CurrentControllerRotation = FRotator(0.0f, 0.0f, 0.0f);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSInteractionComponent> Interaction;
};
