// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/LSWeaponInfoData.h"
#include "LSWeaponData.generated.h"

class UNiagaraSystem;
class UMaterialInterface;
class UUserWidget;

UENUM()
enum class EEquipmentType : uint8
{
    None,
    Main,
    Sub,
    Throwable,
    Melee
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TObjectPtr<ULSWeaponInfoData> WeaponDataAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    EEquipmentType WeaponType = EEquipmentType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSoftClassPtr<AActor> WeaponClass;

    // 장착 시 부착할 소켓 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName SocketName = NAME_None;

    // 아이콘 에셋은 소프트 레퍼런스로 — 하드 레퍼런스 금지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSoftObjectPtr<UTexture2D> Icon;

    // 총구 발사 이펙트 (Niagara)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    TSoftObjectPtr<UNiagaraSystem> MuzzleEffect;

    // 탄착 지점 이펙트 (Niagara)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    TSoftObjectPtr<UNiagaraSystem> ImpactEffect;

    // 탄착 지점 데칼 (Decal Material)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    TSoftObjectPtr<UMaterialInterface> ImpactDecal;

    // 데칼 크기 (박스 half-extent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FVector DecalSize = FVector(8.f);

    // 데칼 수명(초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float DecalLifeSpan = 10.f;

    // 조준선 위젯 클래스 (텍스처 등 비주얼은 BP에서 처리)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
    TSoftClassPtr<UUserWidget> CrosshairWidgetClass;
};