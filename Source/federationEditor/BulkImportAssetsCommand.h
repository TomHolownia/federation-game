// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UToolMenu;

/**
 * Registers the "Import Assets" editor command.
 * Scans Config/ImportSource/Human/ for FBX (and optionally OBJ, GLTF) and imports them
 * into Content/Characters/Human. Tools -> Federation -> Import Assets.
 * Modular and scalable for future bulk import from arbitrary folders.
 */
class FBulkImportAssetsCommand
{
public:
	static void Register();
	static void Unregister();

	/** Run import from fixed MVP paths: Config/ImportSource/Human -> /Game/Characters/Human */
	static void Execute();

	/** Returns list of file paths under SourceDir with supported extensions (.fbx, .obj, .gltf). Used by Execute and by tests. */
	static TArray<FString> GetFilesToImportFromDirectory(const FString& SourceDir);

private:
	static void RegisterMenus();
	static FString GetImportSourceDir();
	static FString GetImportDestinationPath();
};
