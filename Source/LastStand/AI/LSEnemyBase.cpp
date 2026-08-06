// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LSEnemyBase.h"
#include "Character/Components/LSStatComponent.h"

ALSEnemyBase::ALSEnemyBase()
{

	PrimaryActorTick.bCanEverTick = true;

	// Stat
	Stat = CreateDefaultSubobject<ULSStatComponent>(TEXT("Stat"));
}

ULSStatComponent* ALSEnemyBase::GetStatComponent()
{
	return Stat;
}

void ALSEnemyBase::ApplyDamage(int32 Damage)
{
	if (ULSStatComponent* StatComp = GetStatComponent())
	{
		StatComp->ApplyDamage(Damage);
	}
}

// Called when the game starts or when spawned
void ALSEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (Stat)
	{
		Stat->InitializeStatByEnemyData(EnemyName);
	}
}

// Called every frame
void ALSEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALSEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

