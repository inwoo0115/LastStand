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
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Interface/LSStatComponentInterface.h"
#include "Character/Components/LSStatComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"


void ALSWeaponHitscan::LaunchWeapon()
{
	// 소유 클라이언트 입력에 의해 호출됨
	if (bIsEquipping || bIsReloading || CurrentAmmo == 0)
	{
		return;
	}

	// 즉시 1발 발사 후 RapidFire일 경우 연속 발사
	Fire();
	if (bIsRapidFire)
	{
		GetWorldTimerManager().SetTimer(LaunchTimerHandle, this, &ALSWeaponHitscan::Fire, LaunchIntervalTime, true);
	}
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

	Super::UnEquipped();
}

void ALSWeaponHitscan::ReloadWeapon()
{
	// 소유 클라이언트 입력에 의해 호출됨
	if (bIsEquipping || bIsReloading || CurrentAmmo == MaxAmmo)
	{
		return;
	}

	// 장전 중에는 발사 정지
	GetWorldTimerManager().ClearTimer(LaunchTimerHandle);

	// 반응성: 소유 클라에서 즉시 로컬 재생
	PlayWeaponMontage(EWeaponMontageType::Reload);

	ServerRPCReload();
}

void ALSWeaponHitscan::Fire()
{
	// 탄약 소진/장전 중/장착 중이면 연사 타이머 정지
	if (bIsEquipping || bIsReloading || CurrentAmmo == 0)
	{
		GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
		return;
	}

	// 로컬 연사 가드: LaunchIntervalTime 동안 로컬 이펙트/몽타주 차단
	if (!bLocalFireReady)
	{
		return;
	}

	// 숄더 뷰: 화면 중앙(카메라 전방) 기준으로 트레이스 → 착탄이 화면 중앙에 옴
	const FTransform CameraTransform = GetCurrentOwnerCamera();
	const FVector Start = CameraTransform.GetLocation();
	const FVector AimDir = CameraTransform.GetRotation().GetForwardVector();

	// 탄착군: ShotGroupRadius(사거리 지점 반지름)를 콘 반각으로 환산 후 VRandCone
	const float ConeHalfAngleRad = FMath::Atan2(ShotGroupRadius, static_cast<float>(MaxRange));
	const FVector ShotDir = FMath::VRandCone(AimDir, ConeHalfAngleRad);
	const FVector End = Start + ShotDir * static_cast<float>(MaxRange);

	// 반응성: 소유 클라에서 즉시 로컬 실행 (몽타주 + 예측 히트 데미지 UI)
	PlayWeaponLocalEvent(Start, End);

	// 서버에 발사 요청(권위적 판정)
	ServerRPCFire(Start, End);

	// 로컬 연사 가드 시작 (연사 무기는 LaunchTimerHandle이 페이싱하므로 제외)
	if (!bIsRapidFire)
	{
		bLocalFireReady = false;
		GetWorldTimerManager().SetTimer(LocalFireTimerHandle, this, &ALSWeaponHitscan::OnLocalFireReady, LaunchIntervalTime, false);
	}
}

void ALSWeaponHitscan::OnLocalFireReady()
{
	bLocalFireReady = true;
}

void ALSWeaponHitscan::ServerRPCFire_Implementation(const FVector& TraceStart, const FVector& TraceEnd)
{
	if (bIsEquipping || bIsReloading || CurrentAmmo == 0)
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

	// 타 머신으로 발사 몽타주 전파
	MulticastRPCPlayMontage(EWeaponMontageType::Fire);

	// 권위적 라인 트레이스
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

	// 피격 대상이 stat 인터페이스를 가지면 데미지 적용
	if (bHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<ULSStatComponentInterface>())
		{
			ILSStatComponentInterface* Target = Cast<ILSStatComponentInterface>(HitActor);
			Target->ApplyDamage(Damage);
		}
	}

	// 시각화: 총구 → 착탄점(또는 최대 사거리) 디버그 라인. 전 머신에서 각자 그림
	const FVector EndPoint = bHit ? Hit.ImpactPoint : TraceEnd;
	MulticastRPCDrawFireLine(EndPoint, bHit);

	// 발사 이펙트 전파 (소유 클라 제외 나머지 머신)
	MulticastRPCPlayFireEffects(bHit, Hit.ImpactPoint, bHit ? Hit.ImpactNormal : FVector::ZeroVector);
}

void ALSWeaponHitscan::MulticastRPCDrawFireLine_Implementation(const FVector& EndPoint, bool bHit)
{
	if (GetOwner()->HasAuthority())
	{
		return;
	}
#if ENABLE_DRAW_DEBUG
	if (!WeaponMesh)
	{
		return;
	}

	// 총구 소켓에서 착탄점으로 이어지는 궤적(총열이 화면 중앙으로 수렴)
	const FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleName);
	DrawDebugLine(
		GetWorld(),
		MuzzleLocation,
		EndPoint,
		bHit ? FColor::Red : FColor::Green,
		false,
		1.0f);
#endif
}

void ALSWeaponHitscan::MulticastRPCPlayMontage_Implementation(EWeaponMontageType MontageType)
{
	// 소유(로컬 조종) 클라이언트는 이미 로컬에서 재생했으므로 중복 재생 방지
	ACharacter* OwnerCh = Cast<ACharacter>(GetOwner());
	if (OwnerCh && OwnerCh->IsLocallyControlled())
	{
		return;
	}

	PlayWeaponMontage(MontageType);
}

void ALSWeaponHitscan::MulticastRPCPlayFireEffects_Implementation(bool bHit, FVector_NetQuantize ImpactPoint, FVector_NetQuantizeNormal ImpactNormal)
{
	// 소유(로컬 조종) 클라이언트는 이미 로컬에서 재생했으므로 중복 재생 방지
	ACharacter* OwnerCh = Cast<ACharacter>(GetOwner());
	if (OwnerCh && OwnerCh->IsLocallyControlled())
	{
		return;
	}

	PlayFireEffects(bHit, ImpactPoint, ImpactNormal);
}

void ALSWeaponHitscan::PlayFireEffects(bool bHit, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	// 데디 서버는 렌더링하지 않으므로 스킵
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// 총구 발사 이펙트 — 총구 소켓에 부착 (반동/이동 따라감). 명중 여부 무관하게 재생
	if (MuzzleEffect && WeaponMesh)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleEffect,
			WeaponMesh,
			MuzzleName,
			FVector::ZeroVector,
			FRotator(0.f, 0.f, 0.f),
			EAttachLocation::SnapToTarget,
			true);
	}

	if (!bHit)
	{
		return;
	}

	// 탄착 지점 이펙트 — 표면 노멀 방향으로 정렬
	if (ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactEffect,
			ImpactPoint,
			ImpactNormal.Rotation());
	}

	// 탄착 지점 데칼 — 표면 노멀 방향으로 정렬
	if (ImpactDecal)
	{
		UGameplayStatics::SpawnDecalAtLocation(
			GetWorld(),
			ImpactDecal,
			WeaponData.DecalSize,
			ImpactPoint,
			ImpactNormal.Rotation(),
			WeaponData.DecalLifeSpan);
	}
}

void ALSWeaponHitscan::ServerRPCReload_Implementation()
{
	if (bIsEquipping || bIsReloading || CurrentAmmo == MaxAmmo)
	{
		return;
	}

	bIsReloading = true;

	// 타 머신으로 재장전 몽타주 전파
	MulticastRPCPlayMontage(EWeaponMontageType::Reload);

	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &ALSWeaponHitscan::FinishReload, ReloadIntervalTime, false);
}

void ALSWeaponHitscan::FinishReload()
{
	CurrentAmmo = MaxAmmo;
	bIsReloading = false;
}

void ALSWeaponHitscan::PlayWeaponLocalEvent(const FVector& Start, const FVector& End)
{
	// 로컬에서만 실행 (소유 클라)

	// 애니메이션 몽타주 실행 (항상)
	PlayWeaponMontage(EWeaponMontageType::Fire);

	// 로컬 예측 트레이스 (서버와 동일 채널/무시 규칙)
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	// stat 인터페이스 대상 명중 시, 대상 StatComponent가 데미지 계산 + 브로드캐스트
	if (bHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<ULSStatComponentInterface>())
		{
			ILSStatComponentInterface* Target = Cast<ILSStatComponentInterface>(HitActor);
			if (ULSStatComponent* TargetStat = Target->GetStatComponent())
			{
				TargetStat->CalculateDamage(Damage);   // 무기 Damage(raw) 전달
			}
		}
	}

	// 총구 발사 + 착탄 이펙트/데칼 로컬 재생 (반응성)
	PlayFireEffects(bHit, Hit.ImpactPoint, Hit.ImpactNormal);
}

void ALSWeaponHitscan::InitEquipment()
{
	Super::InitEquipment();

	// 무기 정보 초기화
	MaxAmmo = WeaponData.WeaponDataAsset->MaxAmmo;
	CurrentAmmo = MaxAmmo;
	MaxRange = WeaponData.WeaponDataAsset->MaxRange;
	Damage = WeaponData.WeaponDataAsset->Damage;
	ShotGroupRadius = WeaponData.WeaponDataAsset->ShotGroupRadius;
	LaunchIntervalTime = WeaponData.WeaponDataAsset->LaunchIntervalTime;
	ReloadIntervalTime = WeaponData.WeaponDataAsset->ReloadIntervalTime;
	bIsRapidFire = WeaponData.WeaponDataAsset->bIsRapidFire;
	MuzzleName = WeaponData.WeaponDataAsset->MuzzleName;

	// 발사 이펙트 로드 — 데디 서버는 렌더링 안 하므로 스킵
	if (GetNetMode() != NM_DedicatedServer)
	{
		MuzzleEffect = WeaponData.MuzzleEffect.LoadSynchronous();
		ImpactEffect = WeaponData.ImpactEffect.LoadSynchronous();
		ImpactDecal = WeaponData.ImpactDecal.LoadSynchronous();
	}
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
	DOREPLIFETIME(ALSWeaponHitscan, bIsRapidFire);
}
