using UnrealBuildTool;

public class AbyssDiverUnderWorldEditor : ModuleRules
{
    public AbyssDiverUnderWorldEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] 
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            //"AnimGraph",
            //"BlueprintGraph",
            "KismetCompiler",
            "AnimGraphRuntime",
            "AbyssDiverUnderWorld"
        });

        PublicIncludePaths.AddRange(new string[] 
        { 
            "AbyssDiverUnderWorldEditor"
        });

    }
}