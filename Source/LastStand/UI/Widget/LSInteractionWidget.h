// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LSInteractionWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	void HandleNameUpdate(FName NewName);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> NameTextBlock;

};
