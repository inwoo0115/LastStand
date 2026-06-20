// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/LSDropItem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Character/Components/LSInteractionComponent.h"
#include "Character/Components/LSInventoryComponent.h"
#include "Interface/LSInteractComponentInterface.h"
#include "Interface/LSInventoryComponentInterface.h"
#include "Net/UnrealNetwork.h"


ALSDropItem::ALSDropItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	RootComponent = InteractionBox;
	InteractionBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	bReplicates = true;
	SetReplicateMovement(true);
}

void ALSDropItem::InitItem(FName Name, int32 NewQuantity)
{
	ItemName = Name;
	Quantity = NewQuantity;
}

void ALSDropItem::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionBox)
	{
		InteractionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ALSDropItem::OnBoxBeginOverlap);
		InteractionBox->OnComponentEndOverlap.AddUniqueDynamic(this, &ALSDropItem::OnBoxEndOverlap);
	}
}

void ALSDropItem::Interact(AActor* InteractActor)
{
	if (InteractActor && InteractActor->Implements<ULSInventoryComponentInterface>())
	{
		ILSInventoryComponentInterface* EII = Cast<ILSInventoryComponentInterface>(InteractActor);

		EII->GetInventoryComponent()->AddItemToInventory(ItemName, Quantity);
	}
	Destroy();
}

const FName ALSDropItem::GetItemName()
{
	return ItemName;
}

const FGuid ALSDropItem::GetInstanceID()
{
	return InstanceID;
}

const int32 ALSDropItem::GetQuantity()
{
	return Quantity;
}

void ALSDropItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (HasAuthority() && !InstanceID.IsValid())
	{
		InstanceID = FGuid::NewGuid();
	}
}

void ALSDropItem::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->Implements<ULSInteractComponentInterface>())
	{
		if (HasAuthority())
		{
			bCanInteract = true;

			ILSInteractComponentInterface* ICI = Cast<ILSInteractComponentInterface>(OtherActor);
			ICI->GetInteractionComponent()->AddCandidate(this);
		}
	}
}

void ALSDropItem::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->Implements<ULSInteractComponentInterface>())
	{
		if (HasAuthority())
		{
			bCanInteract = false;

			ILSInteractComponentInterface* ICI = Cast<ILSInteractComponentInterface>(OtherActor);
			ICI->GetInteractionComponent()->RemoveCandidate(this);
		}
	}
}

void ALSDropItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALSDropItem, bCanInteract);
	DOREPLIFETIME(ALSDropItem, InstanceID);
	DOREPLIFETIME(ALSDropItem, Quantity);
	DOREPLIFETIME(ALSDropItem, ItemName);
}
