// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSPlayerEquipmentWidget.h"
#include "UI/LSUIEventSubsystem.h"
#include "Character/Components/LSEquipmentComponent.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

void ULSPlayerEquipmentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 로컬 폰의 장비 컴포넌트 캐싱
	EnsureEquipmentComp();

	// 서브시스템 구독 (장비 배열 변경 / 포커스 변경)
	if (ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>())
	{
		ArrayChangedHandle = Sub->EquipmentArrayChanged.AddUObject(this, &ULSPlayerEquipmentWidget::HandleArrayChanged);
		FocusChangedHandle = Sub->FocusEquipmentChanged.AddUObject(this, &ULSPlayerEquipmentWidget::HandleFocusChanged);
	}

	// 초기 pull: 이미 리플리케이트된 장비가 있으면 즉시 표시
	HandleArrayChanged();
}

void ULSPlayerEquipmentWidget::NativeDestruct()
{
	if (ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>())
	{
		if (ArrayChangedHandle.IsValid())
		{
			Sub->EquipmentArrayChanged.Remove(ArrayChangedHandle);
		}
		if (FocusChangedHandle.IsValid())
		{
			Sub->FocusEquipmentChanged.Remove(FocusChangedHandle);
		}
	}

	Super::NativeDestruct();
}

ULSEquipmentComponent* ULSPlayerEquipmentWidget::EnsureEquipmentComp()
{
	if (EquipmentComp.IsValid())
	{
		return EquipmentComp.Get();
	}

	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		EquipmentComp = OwnerPawn->FindComponentByClass<ULSEquipmentComponent>();
	}

	return EquipmentComp.Get();
}

void ULSPlayerEquipmentWidget::HandleArrayChanged()
{
	if (!EnsureEquipmentComp())
	{
		return;
	}

	UpdateSlot(EEquipmentType::Main, MainIcon, MainNameText);
	UpdateSlot(EEquipmentType::Sub, SubIcon, SubNameText);
	UpdateSlot(EEquipmentType::Throwable, ThrowableIcon, ThrowableNameText);
	UpdateSlot(EEquipmentType::Melee, MeleeIcon, MeleeNameText);

	RefreshFocusOpacity();
}

void ULSPlayerEquipmentWidget::HandleFocusChanged()
{
	RefreshFocusOpacity();
}

void ULSPlayerEquipmentWidget::UpdateSlot(EEquipmentType Type, UImage* Icon, UTextBlock* NameText)
{
	ULSEquipmentComponent* Comp = EnsureEquipmentComp();
	if (!Comp)
	{
		return;
	}

	const TMap<EEquipmentType, TObjectPtr<AActor>> EquipmentMap = Comp->GetEquipments();

	if (const TObjectPtr<AActor>* Found = EquipmentMap.Find(Type))
	{
		if (ALSWeaponBase* WB = Cast<ALSWeaponBase>(*Found))
		{
			const FWeaponData WeaponData = WB->GetWeaponData();

			if (NameText)
			{
				NameText->SetText(FText::FromName(WeaponData.ItemName));
			}
			if (Icon)
			{
				if (UTexture2D* Tex = WeaponData.Icon.LoadSynchronous())
				{
					Icon->SetBrushFromTexture(Tex);
				}
				Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
			return;
		}
	}

	// 빈 칸: 이름 비우고 아이콘 숨김
	if (NameText)
	{
		NameText->SetText(FText::GetEmpty());
	}
	if (Icon)
	{
		Icon->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ULSPlayerEquipmentWidget::RefreshFocusOpacity()
{
	ULSEquipmentComponent* Comp = EnsureEquipmentComp();
	const EEquipmentType FocusType = Comp ? Comp->GetFocusEquipmentType() : EEquipmentType::None;

	ApplySlotOpacity(MainIcon, MainNameText, FocusType == EEquipmentType::Main);
	ApplySlotOpacity(SubIcon, SubNameText, FocusType == EEquipmentType::Sub);
	ApplySlotOpacity(ThrowableIcon, ThrowableNameText, FocusType == EEquipmentType::Throwable);
	ApplySlotOpacity(MeleeIcon, MeleeNameText, FocusType == EEquipmentType::Melee);
}

void ULSPlayerEquipmentWidget::ApplySlotOpacity(UImage* Icon, UTextBlock* NameText, bool bFocused)
{
	const float Opacity = bFocused ? 1.0f : UnfocusedOpacity;
	if (Icon)
	{
		Icon->SetRenderOpacity(Opacity);
	}
	if (NameText)
	{
		NameText->SetRenderOpacity(Opacity);
	}
}
