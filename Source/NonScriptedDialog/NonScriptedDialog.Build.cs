// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NonScriptedDialog : ModuleRules
{
	public NonScriptedDialog(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
            "SlateCore",
            "LlamaCore"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"NonScriptedDialog",
			"NonScriptedDialog/Variant_Platforming",
			"NonScriptedDialog/Variant_Platforming/Animation",
			"NonScriptedDialog/Variant_Combat",
			"NonScriptedDialog/Variant_Combat/AI",
			"NonScriptedDialog/Variant_Combat/Animation",
			"NonScriptedDialog/Variant_Combat/Gameplay",
			"NonScriptedDialog/Variant_Combat/Interfaces",
			"NonScriptedDialog/Variant_Combat/UI",
			"NonScriptedDialog/Variant_SideScrolling",
			"NonScriptedDialog/Variant_SideScrolling/AI",
			"NonScriptedDialog/Variant_SideScrolling/Gameplay",
			"NonScriptedDialog/Variant_SideScrolling/Interfaces",
			"NonScriptedDialog/Variant_SideScrolling/UI"
        });

        bEnableUndefinedIdentifierWarnings = false;

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
