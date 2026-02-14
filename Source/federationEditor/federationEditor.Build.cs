// Copyright Federation Game. All Rights Reserved.

using UnrealBuildTool;

public class federationEditor : ModuleRules
{
	public federationEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "federation" });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"LevelEditor",
			"Slate",
			"SlateCore",
			"EditorStyle",
			"Json",
			"JsonUtilities",
			"InputCore"
		});
	}
}
