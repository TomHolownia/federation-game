// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "BulkImportAssetsCommand.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * GetFilesToImportFromDirectory returns only supported extensions (.fbx, .obj, .gltf) and ignores others.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBulkImportGetFilesToImportFiltersByExtension,
	"FederationEditor.BulkImport.GetFilesToImportFiltersByExtension",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FBulkImportGetFilesToImportFiltersByExtension::RunTest(const FString& Parameters)
{
	const FString TempDir = FPaths::ProjectSavedDir() / TEXT("BulkImportTest");
	IFileManager::Get().MakeDirectory(*TempDir, true);

	// Create one supported and one unsupported file
	const FString FbxPath = TempDir / TEXT("dummy.fbx");
	const FString TxtPath = TempDir / TEXT("ignore.txt");
	FFileHelper::SaveStringToFile(TEXT(""), *FbxPath);
	FFileHelper::SaveStringToFile(TEXT(""), *TxtPath);

	TArray<FString> Files = FBulkImportAssetsCommand::GetFilesToImportFromDirectory(TempDir);

	// Cleanup
	IFileManager::Get().Delete(*FbxPath);
	IFileManager::Get().Delete(*TxtPath);
	IFileManager::Get().DeleteDirectory(*TempDir, false, true);

	TestEqual(TEXT("Should return exactly one file (the .fbx)"), Files.Num(), 1);
	if (Files.Num() == 1)
	{
		TestTrue(TEXT("Returned file should be the .fbx"), Files[0].EndsWith(TEXT("dummy.fbx")));
	}
	return true;
}

/**
 * Config/ImportSource/Human directory exists so users can place assets there.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBulkImportSourceDirectoryExists,
	"FederationEditor.BulkImport.SourceDirectoryExists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FBulkImportSourceDirectoryExists::RunTest(const FString& Parameters)
{
	const FString SourceDir = FPaths::ProjectConfigDir() / TEXT("ImportSource") / TEXT("Human");
	TestTrue(
		TEXT("Config/ImportSource/Human should exist (create with Config/ImportSource/Human/.gitkeep)"),
		IFileManager::Get().DirectoryExists(*SourceDir)
	);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
