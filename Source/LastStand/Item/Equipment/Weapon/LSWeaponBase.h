// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equipment/LSEquipmentBase.h"
#include "DataTable/LSWeaponData.h"
#include "Components/TimelineComponent.h"
#include "UI/LSUISubsystem.h"
#include "LSWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSWeaponBase : public ALSEquipmentBase
{
	GENERATED_BODY()

public:
	ALSWeaponBase();
	 
	virtual void LaunchWeapon();

	virtual void ReleaseWeapon();
	
	virtual void Equipped() override;

	virtual void UnEquipped() override;

	virtual void ReloadWeapon();

	virtual void Aim();

	virtual void AimRelease();

	virtual void Tick(float DeltaSeconds) override;

	virtual void ActivateEquipment() override;

	virtual void DeActivateEquipment() override;
	
	FTransform GetCurrentOwnerCamera();

	float GetCurrentOwnerSpringArmLength();

	const FWeaponData GetWeaponData();

	void LinkWeaponAnimClassLayer(UClass* LayerClass);

	void UnLinkWeaponAnimClassLayer();

	void ApplyWeaponAnimLayer();

	// 무기 상황별 몽타주 재생 (오너 캐릭터 메시에서 재생). bReverse=true면 역재생
	void PlayWeaponMontage(EWeaponMontageType MontageType, bool bReverse = false);


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
		Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> WeaponMesh;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 언이큅/파괴 시 넷 롤별 정리를 디스패치
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 파괴 전 넷 롤별 정리 훅 (EndPlay에서 호출)
	virtual void CleanupOnServer();        // 서버 권위 상태 정리
	virtual void CleanupOnClient();        // 비권위(원격 포함) 클라 정리
	virtual void CleanupOnLocalClient();   // 소유 로컬 클라(HUD/카메라) 정리

	// 로컬 정리 공용 헬퍼 (DeActivateEquipment / CleanupOnLocalClient 공용)
	void RemoveCrosshairWidget();   // 주입한 조준선 위젯 해제
	void ResetAimState();           // 에임 타임라인 리셋 → 스프링암 기본 길이 복원

	virtual void InitEquipment() override;

	UPROPERTY(Replicated)
	FWeaponData WeaponData;

	UPROPERTY(ReplicatedUsing=OnRepIsActived)
	bool bIsActived = false;

	UFUNCTION()
	void OnRepIsActived();

	// 장착 가드: Activate 후 EquipIntervalTime 동안 발사/장전 차단 (머신별 로컬 상태)
	void FinishEquip();

	bool bIsEquipping = false;
	FTimerHandle EquipTimerHandle;

	// Aim Timeline
	FTimeline AimTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Timeline)
	TObjectPtr<class UCurveFloat> AimCurveFloat;

	FOnTimelineFloat OnTimelineFloatCallback{};

	UFUNCTION()
	void AimUpdate(float Value);

	void InitAimCurve();

	UPROPERTY(Replicated)
	TObjectPtr<class USpringArmComponent> CachedSpringArm;

	UPROPERTY()
	TObjectPtr<UClass> CurrentAnimLayerClass;

	// 로컬에서 주입한 crosshair 위젯 핸들 (해제용)
	FSlotHandle CrosshairSlotHandle;
};
