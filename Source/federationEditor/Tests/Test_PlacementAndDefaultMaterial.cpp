// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "DefaultGalaxyStarMaterial.h"
#include "PlaceActorsFromDataCommand.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * At least one placement JSON in Config/PlacementData/ must exist and be valid (e.g. GalaxyMapTest.json).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlacementConfigExistsAndIsValidJson,
	"FederationEditor.Placement.ConfigExistsAndIsValidJson",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlacementConfigExistsAndIsValidJson::RunTest(const FString& Parameters)
{
	const FString PlacementDir = FPaths::ProjectConfigDir() / TEXT("PlacementData");
	const FString ConfigPath = PlacementDir / TEXT("GalaxyMapTest.json");

	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *ConfigPath))
	{
		AddError(FString::Printf(TEXT("GalaxyMapTest.json not found or unreadable at %s"), *ConfigPath));
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		AddError(TEXT("GalaxyMapTest.json is not valid JSON"));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* ActorsArray = nullptr;
	if (!Root->TryGetArrayField(TEXT("Actors"), ActorsArray))
	{
		AddError(TEXT("Placement JSON must contain an \"Actors\" array"));
		return false;
	}

	TestTrue(TEXT("Actors array may be empty but must be present"), ActorsArray != nullptr);

	if (ActorsArray && ActorsArray->Num() > 0)
	{
		const TSharedPtr<FJsonObject>* FirstObj = nullptr;
		if ((*ActorsArray)[0]->TryGetObject(FirstObj) && FirstObj->IsValid())
		{
			TestTrue(TEXT("First actor must have Class"), (*FirstObj)->HasField(TEXT("Class")));
			TestTrue(TEXT("First actor must have Location"), (*FirstObj)->HasField(TEXT("Location")));
		}
	}

	return true;
}

/**
 * GetOrCreateDefaultGalaxyStarMaterial must return a valid material in editor context.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDefaultGalaxyStarMaterialReturnsNonNull,
	"FederationEditor.DefaultMaterial.GetOrCreateReturnsNonNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDefaultGalaxyStarMaterialReturnsNonNull::RunTest(const FString& Parameters)
{
	UMaterialInterface* Material = GetOrCreateDefaultGalaxyStarMaterial();
	TestNotNull(TEXT("GetOrCreateDefaultGalaxyStarMaterial must return a valid material"), Material);
	if (Material)
	{
		TestFalse(TEXT("Material should have a valid name"), Material->GetName().IsEmpty());
	}
	return true;
}

/**
 * Place Actors From Data with a SkeletalMeshActor preset (Human.json) spawns an actor whose mesh component has a mesh set (no white dot).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlaceActorsFromDataSkeletalMeshGetsMesh,
	"FederationEditor.Placement.SkeletalMeshActorGetsMeshSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlaceActorsFromDataSkeletalMeshGetsMesh::RunTest(const FString& Parameters)
{
	if (!GEditor)
	{
		AddError(TEXT("Requires editor context"));
		return false;
	}
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("No editor world (open a level first)"));
		return false;
	}

	FPlaceActorsFromDataCommand::Execute(TEXT("Human.json"));

	// Human.json spawns /Script/Engine.SkeletalMeshActor at (0,0,100). Resolve class by path to avoid engine header dependency.
	UClass* SkeletalMeshActorClass = LoadClass<AActor>(nullptr, TEXT("/Script/Engine.SkeletalMeshActor"));
	TestNotNull(TEXT("SkeletalMeshActor class must be loadable"), SkeletalMeshActorClass);
	if (!SkeletalMeshActorClass)
	{
		return false;
	}

	AActor* Found = nullptr;
	const FVector ExpectedLoc(0.0, 0.0, 100.0);
	const float Tolerance = 50.0f;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->IsA(SkeletalMeshActorClass) && FVector::Dist(It->GetActorLocation(), ExpectedLoc) < Tolerance)
		{
			Found = *It;
			break;
		}
	}

	TestNotNull(TEXT("Place Actors From Data Human.json should spawn a SkeletalMeshActor at (0,0,100)"), Found);
	if (!Found)
	{
		return false;
	}

	USkeletalMeshComponent* SMC = Found->FindComponentByClass<USkeletalMeshComponent>();
	TestNotNull(TEXT("SkeletalMeshActor must have a SkeletalMeshComponent"), SMC);
	if (SMC)
	{
		USkeletalMesh* Mesh = Cast<USkeletalMesh>(SMC->GetSkinnedAsset());
		TestNotNull(TEXT("Component must have a mesh set (no white dot)"), Mesh);
	}

	Found->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
