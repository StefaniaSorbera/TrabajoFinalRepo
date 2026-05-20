// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TrabajoFINAL : ModuleRules
{
	public TrabajoFINAL(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TrabajoFINAL",
			"TrabajoFINAL/Variant_Platforming",
			"TrabajoFINAL/Variant_Platforming/Animation",
			"TrabajoFINAL/Variant_Combat",
			"TrabajoFINAL/Variant_Combat/AI",
			"TrabajoFINAL/Variant_Combat/Animation",
			"TrabajoFINAL/Variant_Combat/Gameplay",
			"TrabajoFINAL/Variant_Combat/Interfaces",
			"TrabajoFINAL/Variant_Combat/UI",
			"TrabajoFINAL/Variant_SideScrolling",
			"TrabajoFINAL/Variant_SideScrolling/AI",
			"TrabajoFINAL/Variant_SideScrolling/Gameplay",
			"TrabajoFINAL/Variant_SideScrolling/Interfaces",
			"TrabajoFINAL/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
