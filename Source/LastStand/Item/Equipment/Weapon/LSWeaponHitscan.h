// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "LSWeaponHitscan.generated.h"

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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 발사/장전 헬퍼
	void Fire();          // 소유 클라이언트: 트레이스 계산 후 Server RPC 요청
	void FinishReload();  // 서버: 장전 완료 타이머 콜백

	// Server RPC (소유 클라이언트 → 서버)
	UFUNCTION(Server, Reliable)
	void ServerRPCFire(const FVector& TraceStart, const FVector& TraceEnd);

	UFUNCTION(Server, Reliable)
	void ServerRPCReload();

	// 타이머 / 상태
	FTimerHandle LaunchTimerHandle;
	FTimerHandle ReloadTimerHandle;

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
	
	UPROPERTY(Replicated)
	float ShotGroupRadius;

	UPROPERTY(Replicated)
	float LaunchIntervalTime;

	UPROPERTY(Replicated)
	float ReloadIntervalTime;

	
	
};
