// Copyright Federation Game. All Rights Reserved.

using UnrealBuildTool;

public class federationEditor : ModuleRules
{
	public federationEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// So Tests/ and other subfolders can include module headers by name
		PrivateIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "federation" });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetTools",
			"UnrealEd",
			"LevelEditor",
			"ToolMenus",
			"Slate",
			"SlateCore",
			"EditorStyle",
			"Json",
			"JsonUtilities",
			"InputCore",
			"AssetRegistry",
			"MaterialEditor"
		});
	}
}
