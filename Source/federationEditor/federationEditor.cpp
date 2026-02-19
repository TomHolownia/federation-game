// Copyright Federation Game. All Rights Reserved.

#include "federationEditor.h"
#include "BulkImportAssetsCommand.h"
#include "PlaceActorsFromDataCommand.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ToolMenus.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"

#define LOCTEXT_NAMESPACE "FederationEditor"

void FfederationEditorModule::StartupModule()
{
	FPlaceActorsFromDataCommand::Register();
	FBulkImportAssetsCommand::Register();
}

void FfederationEditorModule::ShutdownModule()
{
	FBulkImportAssetsCommand::Unregister();
	FPlaceActorsFromDataCommand::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FfederationEditorModule, federationEditor)
