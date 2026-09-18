// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMapGeneratorEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// 콘텐츠 브라우저 DataTable 우클릭 메뉴에 "Export Room Bounds" 엔트리 등록
	void RegisterMenus();
};
