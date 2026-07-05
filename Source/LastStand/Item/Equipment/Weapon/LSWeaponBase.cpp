// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "DataTable/LSDataSubsystem.h"
#include "DataTable/LSWeaponData.h"
#include "Components/TimelineComponent.h"
#include "Character/LSCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Data/LSWeaponInfoData.h"
#include "GameFramework/SpringArmComponent.h"


void ALSWeaponBase::LaunchWeapon()
{
}

void ALSWeaponBase::ReleaseWeapon()
{
}

void ALSWeaponBase::Equipped()
{
}

void ALSWeaponBase::UnEquipped()
{
}

void ALSWeaponBase::ReloadWeapon()
{
}

void ALSWeaponBase::Aim()
{
	// 무기에 따른 카메라 위치 변경
	if (AimCurveFloat)
	{
		AimTimeline.Play();
	}
	
}

void ALSWeaponBase::AimRelease()
{
	// 카메라 원 위치
	if (AimCurveFloat)
	{
		AimTimeline.Reverse();
	}
}

void ALSWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Timeline 업데이트
	AimTimeline.TickTimeline(DeltaSeconds);
}

FTransform ALSWeaponBase::GetCurrentOwnerCamera()
{
	ALSCharacterBase *Base = Cast<ALSCharacterBase>(GetOwner());
	if (!Base)
	{
		return FTransform::Identity;
	}

	return Base->GetCurrentCameraTransform();
}

float ALSWeaponBase::GetCurrentOwnerSpringArmLength()
{
	ALSCharacterBase* Base = Cast<ALSCharacterBase>(GetOwner());
	if (!Base)
	{
		return 0.0f;
	}

	return Base->GetCurrentSpringArmLength();
}

const FWeaponData ALSWeaponBase::GetWeaponData()
{
	return WeaponData;
}

void ALSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALSWeaponBase, WeaponData);

}

void ALSWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	InitEquipment();
	InitAimCurve();
}

void ALSWeaponBase::InitEquipment()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	ULSDataSubsystem* Sub = GetOwner()->GetGameInstance()->GetSubsystem<ULSDataSubsystem>();
	if (!Sub)
	{
		return;
	}

	const FWeaponData* ID = Sub->FindWeapon(ItemName);
	if (!ID)
	{
		return;
	}

	// 정보 저장
	WeaponData = *ID;

	// Spring Arm 캐싱
	ALSCharacterBase* Base = Cast<ALSCharacterBase>(GetOwner());
	if (!Base)
	{
		return;
	}
	CachedSpringArm = Base->GetSpringArmComponent();
}

void ALSWeaponBase::AimUpdate(float Value)
{
	CachedSpringArm->TargetArmLength = Value;
}

void ALSWeaponBase::InitAimCurve()
{
	float EndLength = WeaponData.WeaponDataAsset->TargetArmLength;
	float StartLength = GetCurrentOwnerSpringArmLength();

	// AimTimeline 초기화
	AimCurveFloat = NewObject<UCurveFloat>(this, TEXT("AimCurveFloat"));
	FRichCurve& RichCurve = AimCurveFloat->FloatCurve;
	RichCurve.AddKey(0.0f, StartLength); // 시작 시점
	RichCurve.AddKey(0.1f, EndLength); // 끝 시점
	OnTimelineFloatCallback.BindUFunction(this, FName("AimUpdate"));
	AimTimeline.AddInterpFloat(AimCurveFloat, OnTimelineFloatCallback);
	AimTimeline.SetLooping(false);
	AimTimeline.SetPlayRate(1.0f);
}
