// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxVolume.h"
#include "Components/BoxComponent.h"

ABoxVolume::ABoxVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = BoxComponent;

	// 생성 시점 오버랩 판정은 export된 바운즈로 하므로 런타임 콜리전 불필요 — 순수 마커
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComponent->SetBoxExtent(FVector(500.0f));
}
