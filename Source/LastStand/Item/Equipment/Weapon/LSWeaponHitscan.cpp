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
#include "Character/Components/LSHitboxComponent.h"
#include "Character/Components/LSServerSideRewindComponent.h"
#include "GameFramework/GameStateBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/LSUIEventSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Character/Components/LSEquipmentComponent.h"
#include "Character/Components/LSInventoryComponent.h"
#include "Engine/Engine.h"


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
	// 서버 권위: 탄창에 남은 탄약을 인벤토리로 반환 (파괴로 CurrentAmmo가 사라지기 전)
	if (HasAuthority() && CurrentAmmo > 0 && WeaponData.AmmoName != NAME_None)
	{
		if (ULSInventoryComponent* IC = GetOwner()->GetComponentByClass<ULSInventoryComponent>())
		{
			IC->AddItemToInventory(WeaponData.AmmoName, static_cast<int32>(CurrentAmmo));
		}
	}

	// 타이머 등 정리는 파괴 시 EndPlay → CleanupOnServer/LocalClient에서 넷 롤별로 처리
	Super::UnEquipped();
}

void ALSWeaponHitscan::ActivateEquipment()
{
	Super::ActivateEquipment();

	// 베이스가 무기 정보 위젯을 주입한 직후 초기 탄약 전파 (로컬 플레이어 HUD)
	BroadcastAmmoToUI();
}

void ALSWeaponHitscan::OnRep_MaxAmmo()
{
	BroadcastAmmoToUI();
}

void ALSWeaponHitscan::OnRep_CurrentAmmo()
{
	BroadcastAmmoToUI();
}

void ALSWeaponHitscan::BroadcastAmmoToUI()
{
	// 적(AI)·원격 플레이어·데디 서버 제외: 로컬 조종 + 플레이어 조종 폰만 (HUD 소유 클라)
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled() || !OwnerPawn->IsPlayerControlled())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULSUIEventSubsystem* UISub = GI->GetSubsystem<ULSUIEventSubsystem>())
		{
			UISub->AmmoEvent.Broadcast(static_cast<int32>(CurrentAmmo), static_cast<int32>(MaxAmmo));
		}
	}
}

void ALSWeaponHitscan::CleanupOnServer()
{
	Super::CleanupOnServer();

	// 서버 장전 타이머 정리
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
}

void ALSWeaponHitscan::CleanupOnLocalClient()
{
	Super::CleanupOnLocalClient();

	// 로컬 연사/연사 가드 타이머 정리
	GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
	GetWorldTimerManager().ClearTimer(LocalFireTimerHandle);
}

void ALSWeaponHitscan::ReloadWeapon()
{
	// 소유 클라이언트 입력에 의해 호출됨. 예비 탄약이 없으면 장전 불가
	if (bIsEquipping || bIsReloading || CurrentAmmo == MaxAmmo || GetReserveAmmo() <= 0)
	{
		return;
	}

	// 로컬 화면 디버그 로그 (소유 클라 입력 경로에서만 실행)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
			FString::Printf(TEXT("Reload: Current=%u/%u, Reserve=%d"), CurrentAmmo, MaxAmmo, GetReserveAmmo()));
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

	// 반응성: 소유 클라에서 즉시 로컬 실행 (몽타주 + 예측 히트 데미지 UI). 로컬 명중 적 반환
	AActor* LocalHit = PlayWeaponLocalEvent(Start, End);

	// 발사 시각(서버 시간 추정) — 서버 SSR 리와인드 기준
	float FireTime = 0.0f;
	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		FireTime = GS->GetServerWorldTimeSeconds();
	}

	// 서버에 발사 요청: 로컬 명중 정보 + 발사 시각 전달 → 서버 SSR 재검증
	ServerRPCFire(Start, End, LocalHit, FireTime);

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

void ALSWeaponHitscan::ServerRPCFire_Implementation(const FVector& TraceStart, const FVector& TraceEnd, AActor* HitActor, float Timestamp)
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

	// 클라가 로컬 명중을 신고한 경우, 서버 리와인드(SSR)로 재검증 후 데미지
	if (HitActor)
	{
		if (ULSServerSideRewindComponent* SSR = HitActor->GetComponentByClass<ULSServerSideRewindComponent>())
		{
			if (ULSHitboxComponent* Hitbox = SSR->ConfirmHit(TraceStart, TraceEnd, Timestamp))
			{
				Hitbox->ProcessServerHit(Damage);
			}
		}
	}

	// 시각 전용 트레이스(Visibility): 착탄점/이펙트/디버그 라인용 (데미지와 분리)
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

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
	// 서버 권위 가드: 예비 탄약이 없으면 장전 불가 (클라 예측과 desync 방지)
	if (bIsEquipping || bIsReloading || CurrentAmmo == MaxAmmo || GetReserveAmmo() <= 0)
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
	bIsReloading = false;

	// 부족분 계산 (uint32 언더플로 방지 위해 int32 캐스팅)
	const int32 Needed = static_cast<int32>(MaxAmmo) - static_cast<int32>(CurrentAmmo);
	if (Needed <= 0)
	{
		return;
	}

	// 보유 예비 탄약만큼만 장전 (부족하면 있는 개수만)
	const FName AmmoName = WeaponData.AmmoName;
	const int32 Available = GetReserveAmmo();
	const int32 ReloadAmount = FMath::Min(Needed, Available);
	if (ReloadAmount <= 0)
	{
		return;
	}

	// 탄창 충전 (Replicated → OnRep_CurrentAmmo → 탄창 HUD 자동 갱신)
	CurrentAmmo += static_cast<uint32>(ReloadAmount);

	// 인벤토리에서 소모 → 델리게이트가 캐시 차감 + 예비 HUD 갱신 처리
	if (ULSInventoryComponent* IC = GetOwner()->GetComponentByClass<ULSInventoryComponent>())
	{
		IC->AddDeltaToItem(AmmoName, -ReloadAmount);
	}
}

int32 ALSWeaponHitscan::GetReserveAmmo() const
{
	if (const AActor* OwnerActor = GetOwner())
	{
		if (const ULSEquipmentComponent* EC = OwnerActor->GetComponentByClass<ULSEquipmentComponent>())
		{
			return EC->GetAmmoCount(WeaponData.AmmoName);
		}
	}
	return 0;
}

AActor* ALSWeaponHitscan::PlayWeaponLocalEvent(const FVector& Start, const FVector& End)
{
	// 로컬에서만 실행 (소유 클라)

	// 애니메이션 몽타주 실행 (항상)
	PlayWeaponMontage(EWeaponMontageType::Fire);

	// 로컬 예측 트레이스: Hitscan 채널로 Enemy 프로파일 히트박스만 판정
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, HitscanTraceChannel, Params);

	// 피격 컴포넌트가 히트박스면 부위 배율 적용 후 owner StatComponent->CalculateDamage로 연결
	AActor* HitEnemy = nullptr;
	if (bHit)
	{
		if (ULSHitboxComponent* Hitbox = Cast<ULSHitboxComponent>(Hit.GetComponent()))
		{
			Hitbox->ProcessLocalHit(Damage);   // 무기 Damage(raw) 전달
			HitEnemy = Hitbox->GetOwner();      // 서버 SSR 검증용 피격 적
		}
	}

	// 총구 발사 + 착탄 이펙트/데칼 로컬 재생 (반응성)
	PlayFireEffects(bHit, Hit.ImpactPoint, Hit.ImpactNormal);

	return HitEnemy;
}

void ALSWeaponHitscan::InitEquipment()
{
	Super::InitEquipment();

	// 무기 정보 초기화
	MaxAmmo = WeaponData.WeaponDataAsset->MaxAmmo;
	CurrentAmmo = WeaponData.WeaponDataAsset->CurrentAmmo;
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
