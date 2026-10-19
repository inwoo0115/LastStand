// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Data/LSWeaponInfoData.h"
#include "LSAnimNotify_WeaponBase.generated.h"

class APawn;
class ALSWeaponBase;

// 무기 몽타주 노티파이 베이스. 실행할 머신만 걸러내고 실제 재생은 포커스 무기의 훅에 위임
UCLASS(Abstract)
class LASTSTAND_API ULSAnimNotify_WeaponBase : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	// 무기에 넘길 사운드/이펙트/소켓
	UPROPERTY(EditAnywhere, Category = "Weapon")
	FWeaponNotifyEffect Effect;

	// 이 머신에서 실행할지 여부 (파생에서 넷 롤 기준 정의)
	virtual bool ShouldExecute(const APawn* OwnerPawn) const { return false; }

	// 무기 훅 호출 (파생에서 정의)
	virtual void ExecuteOnWeapon(ALSWeaponBase* Weapon) const {}

	// 타임라인 표시용 접두어
	virtual FString GetNotifyPrefix() const { return TEXT("Weapon"); }
};
