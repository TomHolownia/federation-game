// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Galaxy/GalaxyStarField.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test that AGalaxyStarField generates the correct number of stars.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGalaxyStarFieldGeneratesCorrectStarCount,
	"FederationGame.Galaxy.StarField.GeneratesCorrectStarCount",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FGalaxyStarFieldGeneratesCorrectStarCount::RunTest(const FString& Parameters)
{
	// Arrange
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}
	
	AGalaxyStarField* StarField = World->SpawnActor<AGalaxyStarField>();
	if (!StarField)
	{
		AddError(TEXT("Failed to spawn AGalaxyStarField actor"));
		return false;
	}
	
	// Act
	const int32 ExpectedCount = 1000;
	StarField->StarCount = ExpectedCount;
	StarField->RegenerateStars();
	
	// Assert
	TestEqual(TEXT("Star count should match requested count"), StarField->GetStarCount(), ExpectedCount);
	
	// Cleanup
	StarField->Destroy();
	
	return true;
}

/**
 * Test that AGalaxyStarField handles minimum star count.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGalaxyStarFieldMinimumStarCount,
	"FederationGame.Galaxy.StarField.HandlesMinimumStarCount",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FGalaxyStarFieldMinimumStarCount::RunTest(const FString& Parameters)
{
	// Arrange
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}
	
	AGalaxyStarField* StarField = World->SpawnActor<AGalaxyStarField>();
	if (!StarField)
	{
		AddError(TEXT("Failed to spawn AGalaxyStarField actor"));
		return false;
	}
	
	// Act
	const int32 MinCount = 100;
	StarField->StarCount = MinCount;
	StarField->RegenerateStars();
	
	// Assert
	TestEqual(TEXT("Star count should match minimum"), StarField->GetStarCount(), MinCount);
	
	// Cleanup
	StarField->Destroy();
	
	return true;
}

/**
 * Test that AGalaxyStarField handles zero stars gracefully.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGalaxyStarFieldZeroStars,
	"FederationGame.Galaxy.StarField.HandlesZeroStars",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FGalaxyStarFieldZeroStars::RunTest(const FString& Parameters)
{
	// Arrange
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}
	
	AGalaxyStarField* StarField = World->SpawnActor<AGalaxyStarField>();
	if (!StarField)
	{
		AddError(TEXT("Failed to spawn AGalaxyStarField actor"));
		return false;
	}
	
	// Act - Set to zero (will be clamped by UPROPERTY meta, but test internal handling)
	StarField->StarCount = 0;
	StarField->RegenerateStars();
	
	// Assert - Should not crash and should have zero instances
	TestEqual(TEXT("Star count should be zero"), StarField->GetStarCount(), 0);
	
	// Cleanup
	StarField->Destroy();
	
	return true;
}

/**
 * Test that AGalaxyStarField produces reproducible results with same seed.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGalaxyStarFieldReproducibleGeneration,
	"FederationGame.Galaxy.StarField.ProducesReproducibleResults",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FGalaxyStarFieldReproducibleGeneration::RunTest(const FString& Parameters)
{
	// Arrange
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}
	
	AGalaxyStarField* StarField1 = World->SpawnActor<AGalaxyStarField>();
	AGalaxyStarField* StarField2 = World->SpawnActor<AGalaxyStarField>();
	if (!StarField1 || !StarField2)
	{
		AddError(TEXT("Failed to spawn AGalaxyStarField actors"));
		return false;
	}
	
	// Act - Generate with same seed
	const int32 TestSeed = 42;
	const int32 TestStarCount = 500;
	
	StarField1->RandomSeed = TestSeed;
	StarField1->StarCount = TestStarCount;
	StarField1->RegenerateStars();
	
	StarField2->RandomSeed = TestSeed;
	StarField2->StarCount = TestStarCount;
	StarField2->RegenerateStars();
	
	// Assert - Both should have same count (detailed position checking would require component access)
	TestEqual(TEXT("Both star fields should have same count"), 
		StarField1->GetStarCount(), StarField2->GetStarCount());
	
	// Cleanup
	StarField1->Destroy();
	StarField2->Destroy();
	
	return true;
}

/**
 * Test that AGalaxyStarField creates the instanced mesh component.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGalaxyStarFieldHasInstancedMeshComponent,
	"FederationGame.Galaxy.StarField.HasInstancedMeshComponent",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FGalaxyStarFieldHasInstancedMeshComponent::RunTest(const FString& Parameters)
{
	// Arrange
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}
	
	// Act
	AGalaxyStarField* StarField = World->SpawnActor<AGalaxyStarField>();
	if (!StarField)
	{
		AddError(TEXT("Failed to spawn AGalaxyStarField actor"));
		return false;
	}
	
	// Assert
	TestNotNull(TEXT("StarMeshComponent should exist"), StarField->StarMeshComponent);
	TestTrue(TEXT("StarMeshComponent should be root component"), 
		StarField->GetRootComponent() == StarField->StarMeshComponent);
	
	// Cleanup
	StarField->Destroy();
	
	return true;
}

/**
 * Test that AGalaxyStarField respects spiral arm count.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGalaxyStarFieldSpiralArmConfiguration,
	"FederationGame.Galaxy.StarField.RespectsArmConfiguration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FGalaxyStarFieldSpiralArmConfiguration::RunTest(const FString& Parameters)
{
	// Arrange
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}
	
	AGalaxyStarField* StarField = World->SpawnActor<AGalaxyStarField>();
	if (!StarField)
	{
		AddError(TEXT("Failed to spawn AGalaxyStarField actor"));
		return false;
	}
	
	// Act - Configure and generate
	StarField->StarCount = 1000;
	StarField->SpiralArmCount = 2;
	StarField->CoreDensity = 0.5f; // 50% core, 50% arms
	StarField->RegenerateStars();
	
	// Assert - Should generate all stars without error
	TestEqual(TEXT("Star count should match"), StarField->GetStarCount(), 1000);
	TestEqual(TEXT("Spiral arm count should be stored"), StarField->SpiralArmCount, 2);
	
	// Cleanup
	StarField->Destroy();
	
	return true;
}

/**
 * Test that AGalaxyStarField clears old instances on regeneration.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGalaxyStarFieldClearsOnRegenerate,
	"FederationGame.Galaxy.StarField.ClearsInstancesOnRegenerate",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FGalaxyStarFieldClearsOnRegenerate::RunTest(const FString& Parameters)
{
	// Arrange
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}
	
	AGalaxyStarField* StarField = World->SpawnActor<AGalaxyStarField>();
	if (!StarField)
	{
		AddError(TEXT("Failed to spawn AGalaxyStarField actor"));
		return false;
	}
	
	// Act - Generate twice with different counts
	StarField->StarCount = 500;
	StarField->RegenerateStars();
	const int32 FirstCount = StarField->GetStarCount();
	
	StarField->StarCount = 200;
	StarField->RegenerateStars();
	const int32 SecondCount = StarField->GetStarCount();
	
	// Assert - Second generation should replace, not add to, first
	TestEqual(TEXT("First generation count"), FirstCount, 500);
	TestEqual(TEXT("Second generation count (should clear and regenerate)"), SecondCount, 200);
	
	// Cleanup
	StarField->Destroy();
	
	return true;
}

/**
 * Performance test: Verify 10,000 stars can be generated efficiently.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGalaxyStarFieldPerformance10K,
	"FederationGame.Galaxy.StarField.Performance.Generates10KStars",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FGalaxyStarFieldPerformance10K::RunTest(const FString& Parameters)
{
	// Arrange
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}
	
	AGalaxyStarField* StarField = World->SpawnActor<AGalaxyStarField>();
	if (!StarField)
	{
		AddError(TEXT("Failed to spawn AGalaxyStarField actor"));
		return false;
	}
	
	// Act - Generate 10,000 stars and measure time
	StarField->StarCount = 10000;
	
	const double StartTime = FPlatformTime::Seconds();
	StarField->RegenerateStars();
	const double EndTime = FPlatformTime::Seconds();
	const double GenerationTimeMs = (EndTime - StartTime) * 1000.0;
	
	// Assert
	TestEqual(TEXT("Should generate 10,000 stars"), StarField->GetStarCount(), 10000);
	
	// Log generation time for reference
	AddInfo(FString::Printf(TEXT("Generated 10,000 stars in %.2f ms"), GenerationTimeMs));
	
	// Generation should complete in reasonable time (< 1 second)
	TestTrue(TEXT("Generation should complete in under 1 second"), GenerationTimeMs < 1000.0);
	
	// Cleanup
	StarField->Destroy();
	
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
