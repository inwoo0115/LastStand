// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Equipment/Weapon/LSWeaponHitscan.h"
#include "LSWeaponHitscan.h"
#include "Data/LSWeaponInfoData.h"
#include "DataTable/LSWeaponData.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"


void ALSWeaponHitscan::LaunchWeapon()
{
	// 소유 클라이언트 입력에 의해 호출됨
	if (bIsReloading || CurrentAmmo == 0)
	{
		return;
	}

	// 즉시 1발 발사 후 LaunchIntervalTime 간격으로 반복(자동 연사)
	Fire();
	GetWorldTimerManager().SetTimer(LaunchTimerHandle, this, &ALSWeaponHitscan::Fire, LaunchIntervalTime, true);
}

void ALSWeaponHitscan::ReleaseWeapon()
{
	// 발사 정지
	GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
}

void ALSWeaponHitscan::UnEquipped()
{
	// 남은 타이머 정리
	GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);

	UnLinkWeaponAnimClassLayer();

	Super::UnEquipped();
}

void ALSWeaponHitscan::ReloadWeapon()
{
	// 소유 클라이언트 입력에 의해 호출됨
	if (bIsReloading || CurrentAmmo == MaxAmmo)
	{
		return;
	}

	// 장전 중에는 발사 정지
	GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
	ServerRPCReload();
}

void ALSWeaponHitscan::Fire()
{
	// 탄약 소진/장전 중이면 연사 타이머 정지
	if (bIsReloading || CurrentAmmo == 0)
	{
		GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
		return;
	}

	// 카메라 기준 트레이스 시작/끝 계산
	const FTransform CameraTransform = GetCurrentOwnerCamera();
	const FQuat CameraRotation = CameraTransform.GetRotation();

	const FVector Start = CameraTransform.GetLocation();
	const FVector Forward = CameraRotation.GetForwardVector();
	FVector End = Start + Forward * MaxRange;

	// 탄착군: MaxRange 지점에서 조준 방향에 수직인 평면의 반지름 ShotGroupRadius 원 안 랜덤 오프셋(균일 분포)
	const FVector Right = CameraRotation.GetRightVector();
	const FVector Up = CameraRotation.GetUpVector();
	const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
	const float Radius = FMath::Sqrt(FMath::FRand()) * ShotGroupRadius;
	End += Right * (FMath::Cos(Angle) * Radius) + Up * (FMath::Sin(Angle) * Radius);

	// 서버에 발사 요청(권위적 판정)
	ServerRPCFire(Start, End);
}

void ALSWeaponHitscan::ServerRPCFire_Implementation(const FVector& TraceStart, const FVector& TraceEnd)
{
	if (bIsReloading || CurrentAmmo == 0)
	{
		return;
	}

	// 경량 연사속도 가드: LaunchIntervalTime보다 빠른 재호출 무시(약간의 허용오차)
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastFireServerTime < LaunchIntervalTime * 0.9f)
	{
		return;
	}
	LastFireServerTime = Now;

	// 탄약 소모
	--CurrentAmmo;

	// 권위적 라인 트레이스
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

	// TODO: bHit 시 Hit.GetActor()에 데미지 적용(데미지/체력 파이프라인 구축 후)

#if ENABLE_DRAW_DEBUG
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		bHit ? Hit.ImpactPoint : TraceEnd,
		bHit ? FColor::Red : FColor::Green,
		false,
		1.0f);
#endif
}

void ALSWeaponHitscan::ServerRPCReload_Implementation()
{
	if (bIsReloading || CurrentAmmo == MaxAmmo)
	{
		return;
	}

	bIsReloading = true;
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &ALSWeaponHitscan::FinishReload, ReloadIntervalTime, false);
}

void ALSWeaponHitscan::FinishReload()
{
	CurrentAmmo = MaxAmmo;
	bIsReloading = false;
}

void ALSWeaponHitscan::InitEquipment()
{
	Super::InitEquipment();

	// 무기 정보 초기화
	MaxAmmo = WeaponData.WeaponDataAsset->MaxAmmo;
	CurrentAmmo = MaxAmmo;
	MaxRange = WeaponData.WeaponDataAsset->MaxRange;
	ShotGroupRadius = WeaponData.WeaponDataAsset->ShotGroupRadius;
	LaunchIntervalTime = WeaponData.WeaponDataAsset->LaunchIntervalTime;
	ReloadIntervalTime = WeaponData.WeaponDataAsset->ReloadIntervalTime;

	// Animation Layer 설정
	UClass* LayerClass = WeaponData.WeaponDataAsset->AnimLayerClass.LoadSynchronous();
	LinkWeaponAnimClassLayer(LayerClass);
}

void ALSWeaponHitscan::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALSWeaponHitscan, MaxAmmo);
	DOREPLIFETIME(ALSWeaponHitscan, CurrentAmmo);
	DOREPLIFETIME(ALSWeaponHitscan, MaxRange);
	DOREPLIFETIME(ALSWeaponHitscan, ShotGroupRadius);
	DOREPLIFETIME(ALSWeaponHitscan, LaunchIntervalTime);
	DOREPLIFETIME(ALSWeaponHitscan, ReloadIntervalTime);
	DOREPLIFETIME(ALSWeaponHitscan, bIsReloading);
}
