// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class AbyssDiverUnderWorld : ModuleRules
{
	public AbyssDiverUnderWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
            "Core"
            , "CoreUObject"
            , "Engine"
            , "InputCore"
            , "EnhancedInput"
            , "AIModule"
            , "UMG"
            , "OnlineSubsystem"
            , "OnlineSubsystemUtils"
            , "OnlineSubsystemSteam"
            , "AdvancedSessions"
            , "AdvancedSteamSessions"
            , "Slate"
            , "SlateCore"
            , "NetCore"
            , "Niagara"
            , "NavigationSystem"
            , "AnimGraphRuntime"
            , "GameplayTags"
            , "Voice"
            , "NavigationSystem"
            , "CableComponent"
            , "AsyncLoadingScreen"
        });
		
        PublicIncludePaths.AddRange(new string[] { "AbyssDiverUnderWorld" });

        if (Target.Type == TargetType.Editor) 
        {
            PublicDependencyModuleNames.Add("UnrealEd");
        }

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
