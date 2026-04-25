using UnrealBuildTool;

public class ChronicleEngineTests : ModuleRules
{
    public ChronicleEngineTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "ChronicleEngine"
        });
    }
}

