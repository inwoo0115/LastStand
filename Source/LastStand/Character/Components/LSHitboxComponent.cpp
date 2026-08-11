// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSHitboxComponent.h"
#include "Interface/LSStatComponentInterface.h"
#include "Character/Components/LSStatComponent.h"

ULSHitboxComponent::ULSHitboxComponent()
{
	// "Enemy" 프로파일: ObjectType=Pawn, Hitscan 채널 Block (Project Settings 정의). 컴포넌트 디테일에서 BP 수정 가능
	SetCollisionProfileName(TEXT("Enemy"));
	SetGenerateOverlapEvents(false);
}

void ULSHitboxComponent::OnRegister()
{
	Super::OnRegister();

	// KeepRelativeTransform → 디자이너가 설정한 상대 오프셋 보존.
	AttachToOwnerMeshSocket();
}

void ULSHitboxComponent::BeginPlay()
{
	Super::BeginPlay();

	AttachToOwnerMeshSocket();
}

void ULSHitboxComponent::SetupHitbox(ELSHitboxType InType, float InMultiplier)
{
	HitboxType = InType;
	DamageMultiplier = InMultiplier;
}

void ULSHitboxComponent::ProcessLocalHit(int32 RawDamage)
{
	// 부위별 배율 적용
	const int32 FinalDamage = FMath::RoundToInt(RawDamage * DamageMultiplier);

	// owner가 보유한 StatComponent로 전달 → CalculateDamage
	AActor* Owner = GetOwner();
	if (Owner && Owner->Implements<ULSStatComponentInterface>())
	{
		if (ULSStatComponent* Stat = Cast<ILSStatComponentInterface>(Owner)->GetStatComponent())
		{
			Stat->CalculateDamage(FinalDamage);
		}
	}
}

void ULSHitboxComponent::ProcessServerHit(int32 RawDamage)
{
	// 부위별 배율 적용
	const int32 FinalDamage = FMath::RoundToInt(RawDamage * DamageMultiplier);

	// owner가 보유한 StatComponent로 전달 → ApplyDamage (권위 체력 감소)
	AActor* Owner = GetOwner();
	if (Owner && Owner->Implements<ULSStatComponentInterface>())
	{
		if (ULSStatComponent* Stat = Cast<ILSStatComponentInterface>(Owner)->GetStatComponent())
		{
			Stat->ApplyDamage(FinalDamage);
		}
	}
}

void ULSHitboxComponent::AttachToOwnerMeshSocket()
{
	AActor* Owner = GetOwner();
	if (!Owner) { return; }

	if (USkeletalMeshComponent* OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
	{
		// BP가 부모를 Capsule로 저장했더라도 강제로 스켈레탈 메시 소켓에 재부착
		if (GetAttachParent() != OwnerMesh || GetAttachSocketName() != SocketName)
		{
			AttachToComponent(OwnerMesh, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
		}
	}
}
