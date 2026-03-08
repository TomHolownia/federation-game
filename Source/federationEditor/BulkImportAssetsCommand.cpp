// Copyright Federation Game. All Rights Reserved.

#include "BulkImportAssetsCommand.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetImportTask.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogBulkImport, Log, All);

#define LOCTEXT_NAMESPACE "BulkImportAssets"

namespace
{
	static FName BulkImportCommandName(TEXT("BulkImportAssets"));

	const TCHAR* SupportedExtensions[] = { TEXT("fbx"), TEXT("obj"), TEXT("gltf") };
	const int32 NumExtensions = UE_ARRAY_COUNT(SupportedExtensions);

	bool IsSupportedExtension(const FString& Filename)
	{
		const FString Ext = FPaths::GetExtension(Filename, false);
		for (int32 i = 0; i < NumExtensions; ++i)
		{
			if (Ext.Equals(SupportedExtensions[i], ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}
}

void FBulkImportAssetsCommand::Register()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&FBulkImportAssetsCommand::RegisterMenus));
}

void FBulkImportAssetsCommand::Unregister()
{
}

TArray<FImportMapping> FBulkImportAssetsCommand::GetImportMappings()
{
	TArray<FImportMapping> Mappings;
	Mappings.Add({ TEXT("Human/Processed"),  TEXT("/Game/Characters/Human") });
	Mappings.Add({ TEXT("Props/Separated"),  TEXT("/Game/Federation/Props") });
	return Mappings;
}

void FBulkImportAssetsCommand::RegisterMenus()
{
	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	if (ToolsMenu)
	{
		FToolMenuSection& Section = ToolsMenu->FindOrAddSection("Federation");
		Section.AddMenuEntry(
			BulkImportCommandName,
			LOCTEXT("ImportAssetsLabel", "Import Assets"),
			LOCTEXT("ImportAssetsTooltip", "Import all processed assets from Config/ImportSource/ subdirectories into Content/."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FBulkImportAssetsCommand::Execute))
		);
	}
}

TArray<FString> FBulkImportAssetsCommand::GetFilesToImportFromDirectory(const FString& SourceDir)
{
	TArray<FString> FilesToImport;
	IFileManager::Get().FindFilesRecursive(FilesToImport, *SourceDir, TEXT("*.*"), true, false);
	FilesToImport.RemoveAll([&](const FString& Path) { return !IsSupportedExtension(Path); });
	return FilesToImport;
}

int32 FBulkImportAssetsCommand::ImportFromMapping(const FImportMapping& Mapping)
{
	const FString SourceDir = FPaths::ProjectConfigDir() / TEXT("ImportSource") / Mapping.SourceSubDir;

	if (!IFileManager::Get().DirectoryExists(*SourceDir))
	{
		UE_LOG(LogBulkImport, Log, TEXT("Skipping (dir not found): %s"), *SourceDir);
		return 0;
	}

	TArray<FString> FilesToImport = GetFilesToImportFromDirectory(SourceDir);
	if (FilesToImport.Num() == 0)
	{
		UE_LOG(LogBulkImport, Log, TEXT("Skipping (no files): %s"), *SourceDir);
		return 0;
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TArray<UAssetImportTask*> Tasks;
	for (const FString& FilePath : FilesToImport)
	{
		UAssetImportTask* Task = NewObject<UAssetImportTask>(GetTransientPackage());
		Task->Filename = FilePath;
		Task->DestinationPath = Mapping.DestinationPath;
		Task->DestinationName = FPaths::GetBaseFilename(FilePath);
		Task->bAutomated = true;
		Task->bReplaceExisting = true;
		Task->bSave = true;
		Tasks.Add(Task);
	}

	AssetTools.ImportAssetTasks(Tasks);

	int32 SuccessCount = 0;
	for (UAssetImportTask* Task : Tasks)
	{
		if (Task->GetObjects().Num() > 0)
		{
			SuccessCount++;
		}
		else
		{
			UE_LOG(LogBulkImport, Warning, TEXT("Import failed: %s"), *Task->Filename);
		}
	}

	UE_LOG(LogBulkImport, Log, TEXT("Imported %d/%d from %s -> %s"), SuccessCount, Tasks.Num(), *SourceDir, *Mapping.DestinationPath);
	return SuccessCount;
}

void FBulkImportAssetsCommand::Execute()
{
	TArray<FImportMapping> Mappings = GetImportMappings();

	int32 TotalImported = 0;
	int32 TotalFiles = 0;

	for (const FImportMapping& Mapping : Mappings)
	{
		const FString SourceDir = FPaths::ProjectConfigDir() / TEXT("ImportSource") / Mapping.SourceSubDir;
		TArray<FString> Files = GetFilesToImportFromDirectory(SourceDir);
		TotalFiles += Files.Num();
		TotalImported += ImportFromMapping(Mapping);
	}

	if (TotalFiles == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("NoFilesAnywhere", "No FBX/OBJ/GLTF files found in any Config/ImportSource/ subdirectory.\n\nRun the Blender process_mesh.py script first to prepare assets."));
		return;
	}

	FString ResultMsg = FString::Printf(TEXT("Imported %d of %d asset(s) across %d source(s)."), TotalImported, TotalFiles, Mappings.Num());
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultMsg));
}

#undef LOCTEXT_NAMESPACE
