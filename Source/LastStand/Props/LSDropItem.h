// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Props/LSPropBase.h"
#include "Interface/LSInteractableInterface.h"
#include "LSDropItem.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSDropItem : public ALSPropBase, public ILSInteractableInterface
{
	GENERATED_BODY()
	
public:
	ALSDropItem();

	void InitItem(FName Name, int32 NewQuantity);

	virtual void BeginPlay() override;

	virtual void Interact(AActor* InteractActor) override;

	virtual const FName GetItemName() override;

	const int32 GetQuantity();

	
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> InteractionBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemName = "Default";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Quantity = 0;

	UPROPERTY(Replicated)
	bool bCanInteract = false;
};
