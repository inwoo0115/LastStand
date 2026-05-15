// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LSPlayerController.h"
#include "UI/LSUISubsystem.h"
#include "UI/LSRootLayoutWidget.h"

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
	}
}

void ALSPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}

void ALSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}
}
