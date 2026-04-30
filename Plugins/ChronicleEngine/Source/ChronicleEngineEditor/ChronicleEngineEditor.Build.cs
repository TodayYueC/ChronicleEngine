using UnrealBuildTool;

public class ChronicleEngineEditor : ModuleRules
{
    public ChronicleEngineEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "ChronicleEngine",
            "GameplayTags"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "AssetTools",
            "DesktopPlatform",
            "EditorFramework",
            "GraphEditor",
            "InputCore",
            "Json",
            "JsonUtilities",
            "PropertyEditor",
            "Slate",
            "SlateCore",
            "UnrealEd"
        });
    }
}
