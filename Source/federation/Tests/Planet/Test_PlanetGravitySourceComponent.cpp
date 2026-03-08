// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Planet/PlanetGravitySourceComponent.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"
#include "Engine/StaticMeshActor.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravitySourceStrengthFallsOffWithDistance,
	"FederationGame.Planet.PlanetGravitySourceComponent.StrengthFallsOffWithDistance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravitySourceStrengthFallsOffWithDistance::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	AStaticMeshActor* Planet = World->SpawnActor<AStaticMeshActor>();
	if (!Planet) { AddError(TEXT("Failed to spawn planet")); return false; }
	Planet->SetActorScale3D(FVector(10.f));

	UPlanetGravitySourceComponent* Source = NewObject<UPlanetGravitySourceComponent>(Planet, TEXT("GravitySource"));
	Source->RegisterComponent();
	Source->SurfaceGravityScale = 1.0f;
	Source->FalloffExponent = 2.0f;

	const float NearStrength = Source->ComputeGravityStrengthAtDistance(600.f);
	const float FarStrength = Source->ComputeGravityStrengthAtDistance(60000.f);
	TestTrue(TEXT("Strength should be stronger near than far"), NearStrength > FarStrength);

	Planet->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravitySourceMaxInfluenceCutoffWorks,
	"FederationGame.Planet.PlanetGravitySourceComponent.MaxInfluenceCutoffWorks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravitySourceMaxInfluenceCutoffWorks::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	AStaticMeshActor* Planet = World->SpawnActor<AStaticMeshActor>();
	if (!Planet) { AddError(TEXT("Failed to spawn planet")); return false; }
	Planet->SetActorScale3D(FVector(10.f));

	UPlanetGravitySourceComponent* Source = NewObject<UPlanetGravitySourceComponent>(Planet, TEXT("GravitySource"));
	Source->RegisterComponent();
	Source->SurfaceGravityScale = 1.0f;
	Source->MaxInfluenceDistanceMultiplier = 2.0f;

	const float Radius = FMath::Max(1.f, Source->GetSourceRadiusUU());
	const float Inside = Source->ComputeGravityStrengthAtDistance(Radius * 1.5f);
	const float Outside = Source->ComputeGravityStrengthAtDistance(Radius * 3.0f);

	TestTrue(TEXT("Inside cutoff should retain non-zero strength"), Inside > 0.f);
	TestTrue(TEXT("Outside cutoff should be zero"), FMath::IsNearlyZero(Outside, KINDA_SMALL_NUMBER));

	Planet->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
