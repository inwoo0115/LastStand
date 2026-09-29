// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "LSStatComponentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULSStatComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LASTSTAND_API ILSStatComponentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual class ULSStatComponent* GetStatComponent() = 0;

	// 이 인터페이스를 가진 객체에 데미지 적용 — 실제 처리는 StatComponent가 담당
	// DamageCauser: 데미지를 넣은 액터 (null 허용)
	virtual void ApplyDamage(int32 Damage, class AActor* DamageCauser) = 0;


};
