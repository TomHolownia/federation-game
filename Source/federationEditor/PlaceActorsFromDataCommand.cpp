// Copyright Federation Game. All Rights Reserved.

#include "PlaceActorsFromDataCommand.h"
#include "DefaultGalaxyStarMaterial.h"
#include "Galaxy/GalaxyStarField.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"
#include "Engine/Selection.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlaceActors, Log, All);

#define LOCTEXT_NAMESPACE "PlaceActorsFromData"

namespace
{
	static FName PlaceActorsFromDataCommandName(TEXT("PlaceActorsFromData"));

	/** Apply JSON key-value pairs to actor properties via reflection. Works for any actor type. */
	void ApplyPropertiesFromJson(AActor* Actor, const TSharedPtr<FJsonObject>& PropsObj)
	{
		if (!Actor || !PropsObj.IsValid()) return;
		UClass* Class = Actor->GetClass();
		for (const auto& Pair : PropsObj->Values)
		{
			FProperty* Prop = Class->FindPropertyByName(FName(*Pair.Key));
			if (!Prop || !Pair.Value.IsValid()) continue;
			const TSharedPtr<FJsonValue>& Val = Pair.Value;
			void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Actor);
			if (FObjectPropertyBase* ObjProp = CastField<FObjectPropertyBase>(Prop))
			{
				if (Val->Type != EJson::String) continue;
				FString Path = Val->AsString();
				if (Path.IsEmpty()) continue; // Don't overwrite with null from empty string
				UClass* ExpectedClass = ObjProp->PropertyClass;
				if (!ExpectedClass) continue;
				UObject* Loaded = LoadObject<UObject>(nullptr, *Path);
				if (Loaded && Loaded->IsA(ExpectedClass))
				{
					ObjProp->SetObjectPropertyValue(ValuePtr, Loaded);
				}
			}
			else if (FNumericProperty* NumProp = CastField<FNumericProperty>(Prop))
			{
				if (NumProp->IsInteger())
				{
					int64 I = (int64)Val->AsNumber();
					NumProp->SetIntPropertyValue(ValuePtr, I);
				}
				else
				{
					double D = Val->AsNumber();
					NumProp->SetFloatingPointPropertyValue(ValuePtr, D);
				}
			}
			else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
			{
				BoolProp->SetPropertyValue(ValuePtr, Val->AsBool());
			}
			else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
			{
				StrProp->SetPropertyValue(ValuePtr, Val->AsString());
			}
			else if (FNameProperty* NameProp = CastField<FNameProperty>(Prop))
			{
				NameProp->SetPropertyValue(ValuePtr, FName(*Val->AsString()));
			}
		}
	}

	/** Build merged properties: Defaults.Properties + actor Properties. Defaults.StarMesh/StarMaterial merged for GalaxyStarField. */
	TSharedPtr<FJsonObject> MergeProperties(const TSharedPtr<FJsonObject>& Root, const TSharedPtr<FJsonObject>& ActorObj, UClass* ActorClass)
	{
		TSharedPtr<FJsonObject> Merged = MakeShared<FJsonObject>();
		const TSharedPtr<FJsonObject>* DefaultsObj = nullptr;
		if (Root->TryGetObjectField(TEXT("Defaults"), DefaultsObj) && DefaultsObj->IsValid())
		{
			const TSharedPtr<FJsonObject>* DefProps = nullptr;
			if ((*DefaultsObj)->TryGetObjectField(TEXT("Properties"), DefProps) && DefProps->IsValid())
			{
				for (const auto& P : (*DefProps)->Values) { Merged->SetField(P.Key, P.Value); }
			}
			if (ActorClass && ActorClass->IsChildOf(AGalaxyStarField::StaticClass()))
			{
				FString S;
				if ((*DefaultsObj)->TryGetStringField(TEXT("StarMesh"), S)) Merged->SetStringField(TEXT("StarMesh"), S);
				if ((*DefaultsObj)->TryGetStringField(TEXT("StarMaterial"), S)) Merged->SetStringField(TEXT("StarMaterial"), S);
			}
		}
		const TSharedPtr<FJsonObject>* ActorProps = nullptr;
		if (ActorObj->TryGetObjectField(TEXT("Properties"), ActorProps) && ActorProps->IsValid())
		{
			for (const auto& P : (*ActorProps)->Values) { Merged->SetField(P.Key, P.Value); }
		}
		// Per-actor StarMesh/StarMaterial (legacy) override merged
		FString S;
		if (ActorObj->TryGetStringField(TEXT("StarMesh"), S)) Merged->SetStringField(TEXT("StarMesh"), S);
		if (ActorObj->TryGetStringField(TEXT("StarMaterial"), S)) Merged->SetStringField(TEXT("StarMaterial"), S);
		return Merged;
	}
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
	// Toolbar (may be off to the right or in overflow >>)
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
	if (ToolbarMenu)
	{
		FToolMenuSection& ToolbarSection = ToolbarMenu->FindOrAddSection("Federation");
		ToolbarSection.AddMenuEntry(
			PlaceActorsFromDataCommandName,
			LOCTEXT("PlaceActorsFromDataLabel", "Place Actors From Data"),
			LOCTEXT("PlaceActorsFromDataTooltip", "Read Config/PlacementData.json and spawn actors in the current level. Use for AI/agent-driven placement."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FPlaceActorsFromDataCommand::Execute))
		);
	}

	// Tools menu (easy to find: Tools -> Place Actors From Data)
	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	if (ToolsMenu)
	{
		FToolMenuSection& ToolsSection = ToolsMenu->AddSection("Federation", LOCTEXT("FederationSection", "Federation"));
		ToolsSection.AddMenuEntry(
			FName(TEXT("PlaceActorsFromDataTools")),
			LOCTEXT("PlaceActorsFromDataLabel", "Place Actors From Data"),
			LOCTEXT("PlaceActorsFromDataTooltip", "Read Config/PlacementData.json and spawn actors in the current level. Use for AI/agent-driven placement."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FPlaceActorsFromDataCommand::Execute))
		);
		ToolsSection.AddMenuEntry(
			FName(TEXT("LogSelectedStarFieldProps")),
			LOCTEXT("LogStarFieldPropsLabel", "Log selected GalaxyStarField properties"),
			LOCTEXT("LogStarFieldPropsTooltip", "Print the selected GalaxyStarField's StarMesh, StarMaterial, StarCount, etc. to Output Log. Use to compare working vs spawned."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FPlaceActorsFromDataCommand::LogSelectedStarFieldProperties))
		);
		ToolsSection.AddMenuEntry(
			FName(TEXT("UseSelectedStarFieldAsDefault")),
			LOCTEXT("UseSelectedAsDefaultLabel", "Use selected GalaxyStarField as placement default"),
			LOCTEXT("UseSelectedAsDefaultTooltip", "Write the selected GalaxyStarField's StarMesh and StarMaterial into Config/PlacementData.json Defaults. Next Place Actors From Data will use them."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FPlaceActorsFromDataCommand::UseSelectedStarFieldAsPlacementDefault))
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
			TSharedPtr<FJsonObject> MergedProps = MergeProperties(Root, *Obj, ActorClass);
			if (MergedProps->Values.Num() > 0)
			{
				ApplyPropertiesFromJson(Spawned, MergedProps);
			}
			AGalaxyStarField* StarField = Cast<AGalaxyStarField>(Spawned);
			if (StarField)
			{
				if (!StarField->StarMesh)
				{
					StarField->StarMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Shape_Sphere.Shape_Sphere"));
				}
				if (!StarField->StarMaterial)
				{
					StarField->StarMaterial = GetOrCreateDefaultGalaxyStarMaterial();
					if (!StarField->StarMaterial)
					{
						// Fallback so we never leave material null (avoids grey spheres)
						StarField->StarMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
						UE_LOG(LogPlaceActors, Warning, TEXT("GetOrCreateDefaultGalaxyStarMaterial() returned null; using engine DefaultMaterial. Check Output Log and consider Tools -> Use selected GalaxyStarField as placement default."));
					}
				}
				StarField->RegenerateStars();
			}
			SpawnedCount++;
		}
	}

	World->MarkPackageDirty();
	FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
		LOCTEXT("Done", "Placed {0} actor(s) from PlacementData.json."),
		FText::AsNumber(SpawnedCount)
	));
}

void FPlaceActorsFromDataCommand::LogSelectedStarFieldProperties()
{
	if (!GEditor)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoEditorLog", "Editor is not available."));
		return;
	}
	USelection* Selection = GEditor->GetSelectedActors();
	if (!Selection || Selection->Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NothingSelectedLog", "Nothing selected. Select a GalaxyStarField in the World Outliner or viewport, then run this again."));
		return;
	}
	AGalaxyStarField* StarField = nullptr;
	for (FSelectionIterator It(*Selection); It; ++It)
	{
		StarField = Cast<AGalaxyStarField>(*It);
		if (StarField) break;
	}
	if (!StarField)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoStarFieldSelected", "Selection does not contain a GalaxyStarField. Select the star field actor, then run this again."));
		return;
	}
	FString StarMeshPath = StarField->StarMesh ? StarField->StarMesh->GetPathName() : TEXT("(null)");
	FString StarMaterialPath = StarField->StarMaterial ? StarField->StarMaterial->GetPathName() : TEXT("(null)");
	UE_LOG(LogPlaceActors, Log, TEXT("=== GalaxyStarField (selected) ==="));
	UE_LOG(LogPlaceActors, Log, TEXT("StarMesh: %s"), *StarMeshPath);
	UE_LOG(LogPlaceActors, Log, TEXT("StarMaterial: %s"), *StarMaterialPath);
	UE_LOG(LogPlaceActors, Log, TEXT("StarCount: %d"), StarField->StarCount);
	UE_LOG(LogPlaceActors, Log, TEXT("GalaxyRadius: %f"), StarField->GalaxyRadius);
	UE_LOG(LogPlaceActors, Log, TEXT("GetStarCount() (instances): %d"), StarField->GetStarCount());
	FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
		LOCTEXT("LoggedProps", "Logged to Output Log (Window -> Developer Tools -> Output Log).\nStarMesh: {0}\nStarMaterial: {1}"),
		FText::FromString(StarMeshPath), FText::FromString(StarMaterialPath)
	));
}

void FPlaceActorsFromDataCommand::UseSelectedStarFieldAsPlacementDefault()
{
	if (!GEditor)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoEditor", "Editor is not available."));
		return;
	}
	USelection* Selection = GEditor->GetSelectedActors();
	if (!Selection || Selection->Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NothingSelected", "Nothing selected.\n\nSelect exactly one GalaxyStarField in the World Outliner or by clicking it in the viewport, then run Tools → Federation → Use selected GalaxyStarField as placement default again."));
		return;
	}
	AGalaxyStarField* StarField = nullptr;
	for (FSelectionIterator It(*Selection); It; ++It)
	{
		StarField = Cast<AGalaxyStarField>(*It);
		if (StarField) break;
	}
	if (!StarField)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoStarFieldInSelection", "Selection does not contain a GalaxyStarField.\n\nSelect the star field actor (the one that displays correctly) in the World Outliner, then run this again."));
		return;
	}
	// Prefer actor's StarMaterial; if not set, use material from the instanced mesh component (user may have set it in Details)
	UMaterialInterface* MaterialToUse = StarField->StarMaterial;
	if (!MaterialToUse && StarField->StarMeshComponent)
	{
		MaterialToUse = StarField->StarMeshComponent->GetMaterial(0);
	}
	if (!StarField->StarMesh || !MaterialToUse)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
			LOCTEXT("StarFieldMissingProps", "The selected GalaxyStarField has no mesh or material (StarMesh: {0}, material: {1}).\n\nSelect the star field that displays correctly. Set Star Mesh and Star Material in Details, or set the material on the Star Mesh Component."),
			FText::FromString(StarField->StarMesh ? StarField->StarMesh->GetName() : TEXT("(not set)")),
			FText::FromString(MaterialToUse ? MaterialToUse->GetName() : TEXT("(not set)"))
		));
		return;
	}
	FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("PlacementData.json");
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *ConfigPath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoPlacementFile", "Could not read PlacementData.json. Create it first with an Actors array."));
		return;
	}
	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InvalidPlacementJson", "PlacementData.json is not valid JSON."));
		return;
	}
	const TSharedPtr<FJsonObject>* Existing = nullptr;
	TSharedPtr<FJsonObject> Defaults;
	if (Root->TryGetObjectField(TEXT("Defaults"), Existing) && Existing->IsValid())
	{
		Defaults = *Existing;
	}
	else
	{
		Defaults = MakeShared<FJsonObject>();
		Root->SetObjectField(TEXT("Defaults"), Defaults);
	}
	Defaults->SetStringField(TEXT("StarMesh"), StarField->StarMesh->GetPathName());
	Defaults->SetStringField(TEXT("StarMaterial"), MaterialToUse->GetPathName());
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
	if (!FFileHelper::SaveStringToFile(OutputString, *ConfigPath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("WriteFailed", "Could not write PlacementData.json."));
		return;
	}
	FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("DefaultsWritten", "PlacementData.json Defaults updated with this star field's StarMesh and StarMaterial. Run Place Actors From Data to spawn using them."));
}

#undef LOCTEXT_NAMESPACE
