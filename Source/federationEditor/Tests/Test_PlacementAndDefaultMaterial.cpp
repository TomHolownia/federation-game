// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "DefaultGalaxyStarMaterial.h"

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

#endif // WITH_DEV_AUTOMATION_TESTS
