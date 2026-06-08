// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSInteractionComponent.h"
#include "LSInteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "Interface/LSInteractableInterface.h"
#include "UI/LSUISubsystem.h"
#include "Net/UnrealNetwork.h" 

ULSInteractionComponent::ULSInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void ULSInteractionComponent::Interact()
{
	AActor* Owner = GetOwner();

	if (!Owner || Candidates.Num() == 0)
	{
		return;
	}

	float Distance = MAX_FLT;
	AActor* SelectedActor = nullptr;

	// 제일 가까운 액터 상호작용
	for (AActor* Candidate : Candidates)
	{
		if (!Candidate) continue;

		float CurrentDistance = Candidate->GetDistanceTo(Owner);
		if (CurrentDistance < Distance)
		{
			Distance = CurrentDistance;
			SelectedActor = Candidate;
		}
	}

	if (SelectedActor && SelectedActor->Implements<ULSInteractableInterface>())
	{
		ILSInteractableInterface* InteractActor = Cast<ILSInteractableInterface>(SelectedActor);
		if (InteractActor)
		{
			InteractActor->Interact(Owner);
		}
	}
}

void ULSInteractionComponent::InteractCertainCandidate(AActor* Candidate)
{
	// 정해진 액터에 상호작용 하는 함수
	if (Candidate && Candidate->Implements<ULSInteractableInterface>())
	{
		ILSInteractableInterface* InteractActor = Cast<ILSInteractableInterface>(Candidate);
		if (InteractActor && GetOwner())
		{
			InteractActor->Interact(GetOwner());
		}
	}
}

void ULSInteractionComponent::AddCandidate(AActor* NewCandidate)
{
	if (NewCandidate->Implements<ULSInteractableInterface>())
	{
		Candidates.Add(NewCandidate);
	}
}

void ULSInteractionComponent::RemoveCandidate(AActor* DeleteCandidate)
{
	if (DeleteCandidate->Implements<ULSInteractableInterface>())
	{
		Candidates.Remove(DeleteCandidate);
	}
}

bool ULSInteractionComponent::CanInteract()
{
	if (Candidates.Num())
	{
		return true;
	}
	return false;
}

void ULSInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (GetOwner()->HasAuthority())
	{
		SetClosestCandidate();
	}
}

void ULSInteractionComponent::SetClosestCandidate()
{
	AActor* Closest = nullptr;
	AActor* Owner = GetOwner();

	if (CanInteract())
	{
		float Distance = MAX_FLT;

		// 제일 가까운 액터 찾기
		for (AActor* Candidate : Candidates)
		{
			if (!Candidate) continue;

			float CurrentDistance = Candidate->GetDistanceTo(Owner);
			if (CurrentDistance < Distance)
			{
				Distance = CurrentDistance;
				Closest = Candidate;
			}
		}
	}

	ClosestCandidate = Closest;
}

const TArray<TObjectPtr<AActor>> ULSInteractionComponent::GetCandidates()
{
	return Candidates;
}

void ULSInteractionComponent::OnClosestCandidateRep()
{
	APawn* Pn = Cast<APawn>(GetOwner());
	if (Pn && Pn->IsLocallyControlled())
	{
		// BroadCast New Name
		if (ClosestCandidate)
		{
			ILSInteractableInterface* II = Cast<ILSInteractableInterface>(ClosestCandidate);
			if (II)
			{
				OnInteractionUpdate.Broadcast(II->GetItemName());
			}
		}
		else
		{
			OnInteractionUpdate.Broadcast(NAME_None);
		}
	}
}

void ULSInteractionComponent::OnInteractionArrayChange()
{
	OnInteractionArrayUpdate.Broadcast();
}

void ULSInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULSInteractionComponent, ClosestCandidate);
	DOREPLIFETIME(ULSInteractionComponent, Candidates);
}

void ULSInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* Pn = Cast<APawn>(GetOwner());

	if (Pn && Pn->IsLocallyControlled())
	{
		ULSUISubsystem* Sub =  Pn->GetGameInstance()->GetSubsystem<ULSUISubsystem>();
		if (Sub)
		{
			Sub->InjectWidgetToSlot(WidgetTag, InteractionWidget);
		}
	}
}
