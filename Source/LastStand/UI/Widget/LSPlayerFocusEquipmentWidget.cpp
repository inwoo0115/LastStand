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
		// 포커스 무기 변경(리플리케이션 도착) 시 아이콘 재-pull
		FocusHandle = Sub->FocusEquipmentChanged.AddUObject(this, &ULSPlayerFocusEquipmentWidget::RefreshIcon);
	}

	// 초기 아이콘 pull (탄약은 무기 ActivateEquipment의 초기 AmmoEvent로 채워짐)
	RefreshIcon();
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

void ULSPlayerFocusEquipmentWidget::HandleAmmoChanged(int32 Current, int32 Max)
{
	if (CurrentAmmoText)
	{
		CurrentAmmoText->SetText(FText::AsNumber(Current));
	}
	if (MaxAmmoText)
	{
		MaxAmmoText->SetText(FText::AsNumber(Max));
	}
}
