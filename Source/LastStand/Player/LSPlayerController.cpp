// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LSPlayerController.h"
#include "UI/LSUISubsystem.h"
#include "UI/LSUIEventSubsystem.h"
#include "UI/LSRootLayoutWidget.h"
#include "EnhancedInputComponent.h"

ALSPlayerController::ALSPlayerController()
{
}

void ALSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 입력 모드 설정 TODO: 나중에 바꾸기
	FInputModeGameOnly GameInput;
	SetInputMode(GameInput);

	// UI RootLayout Add
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULSUISubsystem* Manager = GI->GetSubsystem<ULSUISubsystem>())
		{
			if (LayoutClass)
				Manager->InitializeRootLayout(this, LayoutClass);
		}

		CachedUISubsystem = GI->GetSubsystem<ULSUIEventSubsystem>();
	}
}


void ALSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}
}

void ALSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);

	//Bind Action
	EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &ALSPlayerController::ToggleInventory);
}

void ALSPlayerController::ToggleInventory(const FInputActionValue& Value)
{
	// Inventory 키기
	CachedUISubsystem->InventoryInput.Broadcast();
}
