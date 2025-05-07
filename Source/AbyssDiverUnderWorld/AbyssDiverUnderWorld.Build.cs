// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AbyssDiverUnderWorld : ModuleRules
{
	public AbyssDiverUnderWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PrivateIncludePaths.AddRange(new string[] {
        "AbyssDiverUnderWorld/Framework",
        "AbyssDiverUnderWorld/Character",
        "AbyssDiverUnderWorld/Monster",
        "AbyssDiverUnderWorld/Boss",
        "AbyssDiverUnderWorld/Shop",
        "AbyssDiverUnderWorld/UI",
        "AbyssDiverUnderWorld/Equipment",
        "AbyssDiverUnderWorld/Interactable",
        "AbyssDiverUnderWorld/Inventory",
        "AbyssDiverUnderWorld/LevelSequence"
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
