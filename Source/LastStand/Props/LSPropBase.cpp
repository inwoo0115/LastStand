// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/LSPropBase.h"


// Sets default values
ALSPropBase::ALSPropBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALSPropBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALSPropBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

