// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "LSWeaponHitscan.generated.h"

class UNiagaraSystem;
class UMaterialInterface;

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSWeaponHitscan : public ALSWeaponBase
{
	GENERATED_BODY()
	
public:
	virtual void LaunchWeapon() override;

	virtual void ReleaseWeapon() override;
	
	virtual void UnEquipped() override;

	virtual void ReloadWeapon() override;

protected:
	virtual void InitEquipment() override;

	// 넷 롤별 정리 (무기별 타이머)
	virtual void CleanupOnServer() override;
	virtual void CleanupOnLocalClient() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 발사/장전 헬퍼
	void Fire();
	void FinishReload();

	// 로컬 연사 가드 (LaunchIntervalTime 동안 로컬 이펙트/몽타주 재실행 차단)
	void OnLocalFireReady();

	// 이펙트, UI 등 로컬 실행 헬퍼 (예측 트레이스 구간 전달)
	void PlayWeaponLocalEvent(const FVector& Start, const FVector& End);

	// 총구 + 착탄 이펙트/데칼을 실제로 스폰 (렌더링 머신에서만 실행)
	void PlayFireEffects(bool bHit, const FVector& ImpactPoint, const FVector& ImpactNormal);

	// Server RPC (소유 클라이언트 → 서버)
	UFUNCTION(Server, Reliable)
	void ServerRPCFire(const FVector& TraceStart, const FVector& TraceEnd);

	UFUNCTION(Server, Reliable)
	void ServerRPCReload();

	// Multicast RPC (서버 → 전 머신, 총구→착탄점 디버그 라인)
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCDrawFireLine(const FVector& EndPoint, bool bHit);

	// 서버 → 전 머신 전파. 소유(로컬 조종) 클라는 이미 로컬 재생했으므로 스킵
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCPlayMontage(EWeaponMontageType MontageType);

	// 서버 → 전 머신, 발사 이펙트(총구/착탄/데칼). 소유 클라는 로컬 재생했으므로 스킵
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCPlayFireEffects(bool bHit, FVector_NetQuantize ImpactPoint, FVector_NetQuantizeNormal ImpactNormal);

	// 타이머 / 상태
	FTimerHandle LaunchTimerHandle;
	FTimerHandle ReloadTimerHandle;

	// 로컬 연사 가드
	bool bLocalFireReady = true;
	FTimerHandle LocalFireTimerHandle;

	UPROPERTY(Replicated)
	bool bIsReloading = false;

	// 서버 연사속도 가드용 마지막 발사 시각
	float LastFireServerTime = 0.0f;

	// 아이템 관련 정보
	UPROPERTY(Replicated)
	uint32 MaxAmmo;

	UPROPERTY(Replicated)
	uint32 CurrentAmmo;

	UPROPERTY(Replicated)
	uint32 MaxRange;

	// 발사 데미지 (서버 ServerRPCFire에서만 사용)
	int32 Damage = 0;
	
	UPROPERTY(Replicated)
	float ShotGroupRadius;

	UPROPERTY(Replicated)
	float LaunchIntervalTime;

	UPROPERTY(Replicated)
	float ReloadIntervalTime;

	UPROPERTY(Replicated)
	bool bIsRapidFire;

	UPROPERTY()
	FName MuzzleName;

	// 발사 이펙트 캐시 (InitEquipment에서 로드, 데디 서버는 스킵)
	UPROPERTY()
	TObjectPtr<UNiagaraSystem> MuzzleEffect;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ImpactDecal;
};
