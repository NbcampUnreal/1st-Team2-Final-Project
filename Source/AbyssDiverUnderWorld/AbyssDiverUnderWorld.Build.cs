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
            , "AnimGraphRuntime"
            ,"RHI"
            ,"RenderCore"
            ,"Renderer"
        });
		
        PublicIncludePaths.AddRange(new string[] { "AbyssDiverUnderWorld" });

        if (Target.Type == TargetType.Editor) 
        {
            PublicDependencyModuleNames.AddRange(new string[]{
            "UnrealEd"            
            });
        }
    }
}
