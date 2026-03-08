// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UToolMenu;

struct FImportMapping
{
	FString SourceSubDir;
	FString DestinationPath;
};

/**
 * Registers the "Import Assets" editor command.
 * Scans Config/ImportSource/ subdirectories for FBX/OBJ/GLTF and imports them
 * into matching Content/ destinations. Tools -> Federation -> Import Assets.
 */
class FBulkImportAssetsCommand
{
public:
	static void Register();
	static void Unregister();

	/** Import all assets from all configured source -> destination mappings. */
	static void Execute();

	/** Returns list of file paths under SourceDir with supported extensions (.fbx, .obj, .gltf). Used by Execute and by tests. */
	static TArray<FString> GetFilesToImportFromDirectory(const FString& SourceDir);

	/** Returns the configured import mappings (source subdirs -> content paths). */
	static TArray<FImportMapping> GetImportMappings();

private:
	static void RegisterMenus();
	static int32 ImportFromMapping(const FImportMapping& Mapping);
};
