// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MapGeneratorEditor : ModuleRules
{
	public MapGeneratorEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"MapGenerator"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"ContentBrowser"
			}
			);
	}
}
