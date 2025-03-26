// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CherryHill : ModuleRules
{
	public CherryHill(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayTags" });
	}
}
