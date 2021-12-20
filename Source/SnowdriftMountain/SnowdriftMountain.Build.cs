// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SnowdriftMountain : ModuleRules
{
	public SnowdriftMountain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
