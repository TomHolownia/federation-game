// Copyright Federation Game. All Rights Reserved.

#include "PlaceActorsFromDataCommand.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "PlaceActorsFromData"

namespace
{
	static FName PlaceActorsFromDataCommandName(TEXT("PlaceActorsFromData"));
}

void FPlaceActorsFromDataCommand::Register()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&FPlaceActorsFromDataCommand::RegisterMenus));
}

void FPlaceActorsFromDataCommand::Unregister()
{
	// Startup callbacks are process-lifecycle; no need to unregister on module shutdown.
}

void FPlaceActorsFromDataCommand::RegisterMenus()
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
	if (ToolbarMenu)
	{
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Federation");
		Section.AddMenuEntry(
			PlaceActorsFromDataCommandName,
			LOCTEXT("PlaceActorsFromDataLabel", "Place Actors From Data"),
			LOCTEXT("PlaceActorsFromDataTooltip", "Read Config/PlacementData.json and spawn actors in the current level. Use for AI/agent-driven placement."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FPlaceActorsFromDataCommand::Execute))
		);
	}
}

void FPlaceActorsFromDataCommand::Execute()
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoWorld", "No editor world. Open a level first."));
		return;
	}

	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("PlacementData.json");
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *ConfigPath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
			LOCTEXT("FileNotFound", "Could not read {0}. Create this file with an \"Actors\" array. See docs/technical/ai-workflow-and-galaxy-scale.md."),
			FText::FromString(ConfigPath)
		));
		return;
	}

	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InvalidJson", "PlacementData.json is not valid JSON."));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* ActorsArray = nullptr;
	if (!Root->TryGetArrayField(TEXT("Actors"), ActorsArray) || !ActorsArray || ActorsArray->Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoActors", "PlacementData.json must contain an \"Actors\" array with at least one entry."));
		return;
	}

	int32 SpawnedCount = 0;
	for (const TSharedPtr<FJsonValue>& Entry : *ActorsArray)
	{
		const TSharedPtr<FJsonObject>* Obj;
		if (!Entry->TryGetObject(Obj) || !Obj->IsValid()) continue;

		FString ClassPath;
		if (!(*Obj)->TryGetStringField(TEXT("Class"), ClassPath)) continue;

		UClass* ActorClass = nullptr;
		if (ClassPath.StartsWith(TEXT("/Script/")))
		{
			ActorClass = LoadObject<UClass>(nullptr, *ClassPath);
		}
		if (!ActorClass)
		{
			ActorClass = LoadClass<AActor>(nullptr, *ClassPath);
		}
		if (!ActorClass)
		{
			continue;
		}

		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FVector Scale = FVector::OneVector;

		const TArray<TSharedPtr<FJsonValue>>* LocArray = nullptr;
		if ((*Obj)->TryGetArrayField(TEXT("Location"), LocArray) && LocArray->Num() >= 3)
		{
			Location.X = (*LocArray)[0]->AsNumber();
			Location.Y = (*LocArray)[1]->AsNumber();
			Location.Z = (*LocArray)[2]->AsNumber();
		}

		const TArray<TSharedPtr<FJsonValue>>* RotArray = nullptr;
		if ((*Obj)->TryGetArrayField(TEXT("Rotation"), RotArray) && RotArray->Num() >= 3)
		{
			Rotation.Pitch = (*RotArray)[0]->AsNumber();
			Rotation.Yaw   = (*RotArray)[1]->AsNumber();
			Rotation.Roll  = (*RotArray)[2]->AsNumber();
		}

		const TArray<TSharedPtr<FJsonValue>>* ScaleArray = nullptr;
		if ((*Obj)->TryGetArrayField(TEXT("Scale"), ScaleArray) && ScaleArray->Num() >= 1)
		{
			double S = (*ScaleArray)[0]->AsNumber();
			Scale = FVector(S);
			if (ScaleArray->Num() >= 3)
			{
				Scale.Y = (*ScaleArray)[1]->AsNumber();
				Scale.Z = (*ScaleArray)[2]->AsNumber();
			}
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FTransform Transform(Rotation, Location, Scale);
		AActor* Spawned = World->SpawnActor<AActor>(ActorClass, Transform, SpawnParams);
		if (Spawned)
		{
			SpawnedCount++;
		}
	}

	World->MarkPackageDirty();
	FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
		LOCTEXT("Done", "Placed {0} actor(s) from PlacementData.json."),
		FText::AsNumber(SpawnedCount)
	));
}

#undef LOCTEXT_NAMESPACE
