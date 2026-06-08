// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "LSInteractionComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractionUpdate, FName);
DECLARE_MULTICAST_DELEGATE(FOnInteractionArrayUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTSTAND_API ULSInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULSInteractionComponent();

	void Interact();

	void InteractCertainCandidate(AActor* Candidate);

	void AddCandidate(AActor* NewCandidate);

	void RemoveCandidate(AActor* DeleteCandidate);

	bool CanInteract();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetClosestCandidate();

	const TArray<TObjectPtr<AActor>> GetCandidates();

	// UI Broadcast
	FOnInteractionUpdate OnInteractionUpdate;
	
	FOnInteractionArrayUpdated OnInteractionArrayUpdate;
protected:
	// RPC
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when the game starts
	virtual void BeginPlay() override;

	// OnRep
	UFUNCTION()
	void OnClosestCandidateRep();

	UFUNCTION()
	void OnInteractionArrayChange();

	UPROPERTY(ReplicatedUsing = OnInteractionArrayChange)
	TArray<TObjectPtr<AActor>> Candidates;

	UPROPERTY(ReplicatedUsing=OnClosestCandidateRep)
	TObjectPtr<AActor> ClosestCandidate = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<class UUserWidget> InteractionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FGameplayTag WidgetTag;
};
