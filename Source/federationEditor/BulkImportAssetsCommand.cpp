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

FString FBulkImportAssetsCommand::GetImportSourceDir()
{
	return FPaths::ProjectConfigDir() / TEXT("ImportSource") / TEXT("Human");
}

FString FBulkImportAssetsCommand::GetImportDestinationPath()
{
	return TEXT("/Game/Characters/Human");
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
			LOCTEXT("ImportAssetsTooltip", "Import FBX/OBJ/GLTF from Config/ImportSource/Human into Content/Characters/Human."),
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

void FBulkImportAssetsCommand::Execute()
{
	const FString SourceDir = GetImportSourceDir();
	const FString DestPath = GetImportDestinationPath();

	if (!IFileManager::Get().DirectoryExists(*SourceDir))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
			LOCTEXT("SourceDirMissing", "Source directory does not exist: {0}\n\nCreate it and place one or more FBX (or OBJ/GLTF) files there, then run Import Assets again."),
			FText::FromString(SourceDir)
		));
		return;
	}

	TArray<FString> FilesToImport = GetFilesToImportFromDirectory(SourceDir);

	if (FilesToImport.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
			LOCTEXT("NoFiles", "No FBX, OBJ, or GLTF files found in {0}"),
			FText::FromString(SourceDir)
		));
		return;
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TArray<UAssetImportTask*> Tasks;
	for (const FString& FilePath : FilesToImport)
	{
		UAssetImportTask* Task = NewObject<UAssetImportTask>(GetTransientPackage());
		Task->Filename = FilePath;
		Task->DestinationPath = DestPath;
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
			UE_LOG(LogBulkImport, Warning, TEXT("Import failed or produced no assets: %s"), *Task->Filename);
		}
	}

	FString ResultMsg = FString::Printf(TEXT("Imported %d of %d asset(s) to %s."), SuccessCount, Tasks.Num(), *DestPath);
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultMsg));
}

#undef LOCTEXT_NAMESPACE
