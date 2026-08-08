// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LSPlayerStatWidget.h"
#include "UI/LSUIEventSubsystem.h"
#include "Interface/LSStatComponentInterface.h"
#include "Character/Components/LSStatComponent.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

void ULSPlayerStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// HealthEvent 구독 (LSInventoryWidget/LSDamageLayerWidget 패턴)
	if (ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>())
	{
		HealthDelegateHandle = Sub->HealthEvent.AddUObject(this, &ULSPlayerStatWidget::HandleHealthEvent);
	}

	// 초기값 pull: 구독-초기화 순서와 무관하게 최초 표시 보장
	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		if (ILSStatComponentInterface* StatInterface = Cast<ILSStatComponentInterface>(OwnerPawn))
		{
			if (ULSStatComponent* Stat = StatInterface->GetStatComponent())
			{
				UpdateHealthText(Stat->GetCurrentHealth(), Stat->GetMaxHealth());
			}
		}
	}
}

void ULSPlayerStatWidget::NativeDestruct()
{
	if (ULSUIEventSubsystem* Sub = GetGameInstance()->GetSubsystem<ULSUIEventSubsystem>())
	{
		if (HealthDelegateHandle.IsValid())
		{
			Sub->HealthEvent.Remove(HealthDelegateHandle);
		}
	}

	Super::NativeDestruct();
}

void ULSPlayerStatWidget::HandleHealthEvent(int32 Current, int32 Max)
{
	UpdateHealthText(Current, Max);
}

void ULSPlayerStatWidget::UpdateHealthText(int32 Current, int32 Max)
{
	if (CurrentHealthText)
	{
		CurrentHealthText->SetText(FText::AsNumber(Current));
	}
	if (MaxHealthText)
	{
		MaxHealthText->SetText(FText::AsNumber(Max));
	}
}
