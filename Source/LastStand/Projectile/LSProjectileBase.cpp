// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/LSProjectileBase.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

ALSProjectileBase::ALSProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	// 무한 비행 방지
	InitialLifeSpan = 5.f;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	// Hit 이벤트 활성화
	CollisionBox->SetNotifyRigidBodyCollision(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionBox);
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComponent"));
	TrailComponent->SetupAttachment(CollisionBox);
	TrailComponent->bAutoActivate = false;
}

void ALSProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// 쏜 사람 자기 충돌 방지 (모든 머신 — 클라 이동 시뮬레이션도 막히지 않도록)
	if (AActor* InstigatorActor = GetInstigator())
	{
		CollisionBox->IgnoreActorWhenMoving(InstigatorActor, true);
	}
	if (AActor* OwnerActor = GetOwner())
	{
		CollisionBox->IgnoreActorWhenMoving(OwnerActor, true);
	}

	// 충돌 판정은 서버 권위
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &ALSProjectileBase::OnBoxHit);
	}

	// 데디 서버는 렌더링하지 않으므로 트레일 스킵
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (UNiagaraSystem* Trail = TrailEffect.LoadSynchronous())
		{
			TrailComponent->SetAsset(Trail);
			TrailComponent->Activate(true);
		}
	}
}

void ALSProjectileBase::Destroyed()
{
	// 서버 Destroy → 리플리케이션으로 모든 머신에서 호출되므로 여기서 착탄 연출
	PlayImpactEffect();

	Super::Destroyed();
}

void ALSProjectileBase::OnProjectileHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit)
{
	Destroy();
}

void ALSProjectileBase::OnBoxHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	OnProjectileHit(OtherActor, OtherComp, Hit);
}

void ALSProjectileBase::PlayImpactEffect()
{
	// 데디 서버는 렌더링하지 않으므로 스킵
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UNiagaraSystem* Impact = ImpactEffect.LoadSynchronous())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			Impact,
			GetActorLocation(),
			GetActorRotation());
	}
}
