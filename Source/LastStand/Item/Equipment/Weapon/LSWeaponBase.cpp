// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "DataTable/LSDataSubsystem.h"
#include "DataTable/LSWeaponData.h"
#include "Components/TimelineComponent.h"
#include "Character/LSCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Data/LSWeaponInfoData.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimMontage.h"


ALSWeaponBase::ALSWeaponBase()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);
}

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
	Super::UnEquipped();
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

void ALSWeaponBase::ActivateEquipment()
{
	Super::ActivateEquipment();

	if (WeaponMesh)
	{
		// 일단 No Collision으로 세팅 추후에 콜리전 세팅 할 때 변경
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetVisibility(true);
	}
	bIsActived = true;

	// 장착 가드 시작: EquipIntervalTime 동안 발사/장전 차단 (서버·클라 각자 로컬)
	bIsEquipping = true;
	const float EquipTime = WeaponData.WeaponDataAsset ? WeaponData.WeaponDataAsset->EquipIntervalTime : 0.0f;
	if (EquipTime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(EquipTimerHandle, this, &ALSWeaponBase::FinishEquip, EquipTime, false);
	}
	else
	{
		bIsEquipping = false;
	}
}

void ALSWeaponBase::DeActivateEquipment()
{
	Super::DeActivateEquipment();
	
	if (WeaponMesh)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetVisibility(false);
	}
	bIsActived = false;

	// 장착 가드 정리
	GetWorldTimerManager().ClearTimer(EquipTimerHandle);
	bIsEquipping = false;
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

void ALSWeaponBase::LinkWeaponAnimClassLayer(UClass* LayerClass)
{
	if (!LayerClass)
	{
		return;
	}

	ACharacter* OwnerCh = Cast<ACharacter>(GetOwner());
	if (OwnerCh)
	{
		if (USkeletalMeshComponent* MeshComp = OwnerCh->GetMesh())
		{
			if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			{
				AnimInstance->LinkAnimClassLayers(LayerClass);
			}
		}
		CurrentAnimLayerClass = LayerClass; // 나중에 Unlink 하려면 캐싱해둠
	}
}

void ALSWeaponBase::UnLinkWeaponAnimClassLayer()
{
	// Animation Layer 해제
	ACharacter* OwnerCh = Cast<ACharacter>(GetOwner());
	if (OwnerCh)
	{
		if (USkeletalMeshComponent* MeshComp = OwnerCh->GetMesh())
		{
			if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			{
				AnimInstance->UnlinkAnimClassLayers(CurrentAnimLayerClass);
			}
		}
	}
}

void ALSWeaponBase::ApplyWeaponAnimLayer()
{
	if (!WeaponData.WeaponDataAsset)
	{
		return;
	}

	UClass* LayerClass = WeaponData.WeaponDataAsset->AnimLayerClass.LoadSynchronous();
	LinkWeaponAnimClassLayer(LayerClass);
}

void ALSWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 언이큅/파괴 시 이 무기가 링크한 레이어를 안전하게 해제
	// (포커스 무기든 아니든 동일. 링크한 적 없으면 CurrentAnimLayerClass=null 이라 내부에서 안전한 no-op)
	UnLinkWeaponAnimClassLayer();

	Super::EndPlay(EndPlayReason);
}

void ALSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALSWeaponBase, WeaponData);
	DOREPLIFETIME(ALSWeaponBase, bIsActived);
	DOREPLIFETIME(ALSWeaponBase, CachedSpringArm);
}

void ALSWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	InitEquipment();
	InitAimCurve();
}

void ALSWeaponBase::InitEquipment()
{
	// 애니메이션 세팅을 위해 클라이언트에서도 각각 데이터 저장
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

void ALSWeaponBase::OnRepIsActived()
{
	if (bIsActived)
	{
		UE_LOG(LogTemp, Log, TEXT("ALSWeaponBase::OnRepIs Actived"));

		ActivateEquipment();

		// 서버 제외 모든 클라에서 장착 몽타주 재생 (OnRep은 클라에서만 발화)
		PlayWeaponMontage(EWeaponMontageType::Equip);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ALSWeaponBase::OnRepIs DeActived"));

		DeActivateEquipment();
	}
}

void ALSWeaponBase::FinishEquip()
{
	bIsEquipping = false;
}

void ALSWeaponBase::PlayWeaponMontage(EWeaponMontageType MontageType)
{
	if (!WeaponData.WeaponDataAsset)
	{
		return;
	}

	const TSoftObjectPtr<UAnimMontage>* Found = WeaponData.WeaponDataAsset->WeaponMontages.Find(MontageType);
	if (!Found)
	{
		return;
	}

	UAnimMontage* Montage = Found->LoadSynchronous();
	if (!Montage)
	{
		return;
	}

	ACharacter* OwnerCh = Cast<ACharacter>(GetOwner());
	if (OwnerCh)
	{
		// 캐릭터 Mesh의 AnimInstance에서 재생
		OwnerCh->PlayAnimMontage(Montage);
	}
}

void ALSWeaponBase::AimUpdate(float Value)
{
	if (!CachedSpringArm)
	{
		return;
	}
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
