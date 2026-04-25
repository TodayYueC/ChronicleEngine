using UnrealBuildTool;
using System.Collections.Generic;

public class ChronicleHostEditorTarget : TargetRules
{
    public ChronicleHostEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        CppStandard = CppStandardVersion.Cpp20;
        ExtraModuleNames.Add("ChronicleHost");
    }
}
