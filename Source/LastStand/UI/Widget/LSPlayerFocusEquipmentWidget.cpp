// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSPlayerFocusEquipmentWidget.h"
#include "UI/LSUIEventSubsystem.h"
#include "Character/Components/LSEquipmentComponent.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

void ULSPlayerFocusEquipmentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>())
	{
		// 탄약 갱신 구독
		AmmoHandle = Sub->AmmoEvent.AddUObject(this, &ULSPlayerFocusEquipmentWidget::HandleAmmoChanged);
		// 포커스 무기 변경(리플리케이션 도착) 시 아이콘 + 예비 탄약 재-pull
		FocusHandle = Sub->FocusEquipmentChanged.AddUObject(this, &ULSPlayerFocusEquipmentWidget::HandleFocusEquipmentChanged);
		// 예비 탄약(캐시) 변경 시 재-pull
		ReserveAmmoHandle = Sub->ReserveAmmoChanged.AddUObject(this, &ULSPlayerFocusEquipmentWidget::RefreshReserveAmmo);
	}

	// 초기 pull (탄창 현재량은 무기 ActivateEquipment의 초기 AmmoEvent로 채워짐)
	RefreshIcon();
	RefreshReserveAmmo();
}

void ULSPlayerFocusEquipmentWidget::NativeDestruct()
{
	if (ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>())
	{
		if (AmmoHandle.IsValid())
		{
			Sub->AmmoEvent.Remove(AmmoHandle);
		}
		if (FocusHandle.IsValid())
		{
			Sub->FocusEquipmentChanged.Remove(FocusHandle);
		}
		if (ReserveAmmoHandle.IsValid())
		{
			Sub->ReserveAmmoChanged.Remove(ReserveAmmoHandle);
		}
	}

	Super::NativeDestruct();
}

void ULSPlayerFocusEquipmentWidget::RefreshIcon()
{
	if (!WeaponIcon)
	{
		return;
	}

	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (!OwnerPawn)
	{
		return;
	}

	ULSEquipmentComponent* EquipComp = OwnerPawn->FindComponentByClass<ULSEquipmentComponent>();
	if (!EquipComp)
	{
		return;
	}

	ALSWeaponBase* Focus = EquipComp->GetFocusEquipment();
	if (!Focus)
	{
		return;
	}

	if (UTexture2D* Tex = Focus->GetWeaponData().Icon.LoadSynchronous())
	{
		WeaponIcon->SetBrushFromTexture(Tex);
	}
}

void ULSPlayerFocusEquipmentWidget::RefreshReserveAmmo()
{
	if (!MaxAmmoText)
	{
		return;
	}

	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (!OwnerPawn)
	{
		return;
	}

	ULSEquipmentComponent* EquipComp = OwnerPawn->FindComponentByClass<ULSEquipmentComponent>();
	if (!EquipComp)
	{
		return;
	}

	ALSWeaponBase* Focus = EquipComp->GetFocusEquipment();
	if (!Focus)
	{
		return;
	}

	// 포커스 무기가 사용하는 탄약(FWeaponData::AmmoName)의 캐시 잔량 표시
	const FName AmmoName = Focus->GetWeaponData().AmmoName;
	const int32 Reserve = EquipComp->GetAmmoCount(AmmoName);
	MaxAmmoText->SetText(FText::AsNumber(Reserve));
}

void ULSPlayerFocusEquipmentWidget::HandleFocusEquipmentChanged()
{
	// 포커스 무기가 바뀌면 아이콘과 예비 탄약(탄약 종류)이 함께 달라짐
	RefreshIcon();
	RefreshReserveAmmo();
}

void ULSPlayerFocusEquipmentWidget::HandleAmmoChanged(int32 Current, int32 Max)
{
	// 현재 탄창량만 갱신 (예비 탄약은 RefreshReserveAmmo가 캐시에서 pull)
	if (CurrentAmmoText)
	{
		CurrentAmmoText->SetText(FText::AsNumber(Current));
	}
}
