using UnrealBuildTool;
using System.Collections.Generic;

public class ChronicleHostTarget : TargetRules
{
    public ChronicleHostTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        CppStandard = CppStandardVersion.Cpp20;
        ExtraModuleNames.Add("ChronicleHost");
    }
}
