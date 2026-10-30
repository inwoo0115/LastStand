// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LSProjectileBase.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class LASTSTAND_API ALSProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ALSProjectileBase();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	// 서버에서만 호출. 파생 클래스가 데미지 등을 구현 (기본: Destroy)
	virtual void OnProjectileHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit);

	UFUNCTION()
	void OnBoxHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 착탄 이펙트 재생 (데디 서버 스킵)
	void PlayImpactEffect();

protected:
	// 루트 충돌 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Component")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// 비행 중 트레일 이펙트 (BeginPlay에서 TrailEffect 에셋 세팅 후 활성화)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Component")
	TObjectPtr<UNiagaraComponent> TrailComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Effect")
	TSoftObjectPtr<UNiagaraSystem> TrailEffect;

	// 파괴 시점(착탄)에 재생할 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Effect")
	TSoftObjectPtr<UNiagaraSystem> ImpactEffect;
};
