// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Planet/PlanetGravityComponent.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMeshActor.h"

#if WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Helper: spawn a Character with a PlanetGravityComponent attached.
// We use a plain ACharacter so tests are independent of AFederationCharacter.
// ---------------------------------------------------------------------------
static ACharacter* SpawnCharacterWithGravityComp(UWorld* World, UPlanetGravityComponent*& OutComp)
{
	ACharacter* Char = World->SpawnActor<ACharacter>();
	if (!Char) return nullptr;

	OutComp = NewObject<UPlanetGravityComponent>(Char, TEXT("TestGravityComp"));
	OutComp->RegisterComponent();
	return Char;
}

// ---------------------------------------------------------------------------
// 1. Gravity direction toward tagged planet
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentGravityDirectionTowardPlanet,
	"FederationGame.Planet.PlanetGravityComponent.GravityDirectionTowardPlanet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentGravityDirectionTowardPlanet::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetGravityComponent* Comp = nullptr;
	ACharacter* Char = SpawnCharacterWithGravityComp(World, Comp);
	if (!Char || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	AStaticMeshActor* Planet = World->SpawnActor<AStaticMeshActor>();
	Planet->Tags.Add(FName(TEXT("Planet")));
	Planet->SetActorLocation(FVector::ZeroVector);
	Planet->SetActorScale3D(FVector(10.f));

	Char->SetActorLocation(FVector(0.f, 0.f, 500.f));
	Comp->UpdatePlanetGravity();

	TestTrue(TEXT("Gravity should point toward planet (downward)"),
		FVector::DotProduct(Comp->GravityDir, FVector(0.f, 0.f, -1.f)) > 0.99f);

	Planet->Destroy();
	Char->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 2. Capsule aligns to gravity
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentCapsuleAlignsToGravity,
	"FederationGame.Planet.PlanetGravityComponent.CapsuleAlignsToGravity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentCapsuleAlignsToGravity::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetGravityComponent* Comp = nullptr;
	ACharacter* Char = SpawnCharacterWithGravityComp(World, Comp);
	if (!Char || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->GravityDir = FVector(1.f, 0.f, 0.f);
	Comp->bAlignToGravity = true;

	for (int i = 0; i < 60; ++i)
	{
		Comp->UpdateGravityAlignment(0.05f);
	}

	const FVector ActorUp = Char->GetActorUpVector();
	TestTrue(TEXT("Actor up should oppose gravity direction"),
		FVector::DotProduct(ActorUp, FVector(-1.f, 0.f, 0.f)) > 0.99f);

	Char->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 3. Gravity-relative look updates state
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentLookUpdatesState,
	"FederationGame.Planet.PlanetGravityComponent.LookUpdatesState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentLookUpdatesState::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetGravityComponent* Comp = nullptr;
	ACharacter* Char = SpawnCharacterWithGravityComp(World, Comp);
	if (!Char || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->GravityDir = FVector(0.f, 0.f, 1.f);
	Comp->bViewInitialized = false;
	Comp->ViewPitchRad = 0.f;
	Comp->ViewTangentForward = FVector(1.f, 0.f, 0.f);

	Comp->ApplyLookInput(90.f, 0.f);
	TestTrue(TEXT("After 90 deg yaw, tangent forward should be near +Y"),
		FVector::DotProduct(Comp->ViewTangentForward, FVector(0.f, 1.f, 0.f)) > 0.95f);

	const float PitchBefore = Comp->ViewPitchRad;
	Comp->ApplyLookInput(0.f, 10.f);
	TestTrue(TEXT("Pitch should have increased after positive pitch input"),
		Comp->ViewPitchRad > PitchBefore);

	Char->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 4. Pitch clamped to max
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentPitchClampedToMax,
	"FederationGame.Planet.PlanetGravityComponent.PitchClampedToMax",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentPitchClampedToMax::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetGravityComponent* Comp = nullptr;
	ACharacter* Char = SpawnCharacterWithGravityComp(World, Comp);
	if (!Char || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->GravityDir = FVector(0.f, 0.f, 1.f);
	Comp->MaxLookPitchDegrees = 85.f;

	Comp->ApplyLookInput(0.f, 9999.f);

	const float MaxRad = FMath::DegreesToRadians(85.f);
	TestTrue(TEXT("Pitch should be clamped to max"),
		FMath::Abs(Comp->ViewPitchRad) <= MaxRad + 0.01f);

	Char->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 5. Camera orientation driven by quaternion
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentCameraOrientationQuat,
	"FederationGame.Planet.PlanetGravityComponent.CameraOrientationDrivenByQuat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentCameraOrientationQuat::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetGravityComponent* Comp = nullptr;
	ACharacter* Char = SpawnCharacterWithGravityComp(World, Comp);
	if (!Char || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	// Create a camera root scene component
	USceneComponent* CamRoot = NewObject<USceneComponent>(Char, TEXT("TestCamRoot"));
	CamRoot->SetupAttachment(Char->GetRootComponent());
	CamRoot->SetUsingAbsoluteRotation(true);
	CamRoot->RegisterComponent();

	Comp->SetCameraReferences(CamRoot, nullptr);
	Comp->GravityDir = FVector(0.f, 0.f, 1.f);
	Comp->bAlignToGravity = true;
	Comp->bUseGravityRelativeLook = true;
	Comp->ViewTangentForward = FVector(1.f, 0.f, 0.f);
	Comp->ViewPitchRad = 0.f;
	Comp->bViewInitialized = true;
	Comp->ViewUp = FVector(0.f, 0.f, -1.f);

	Comp->UpdateCameraOrientation();

	const FVector Up = FVector(0.f, 0.f, -1.f);
	const FQuat ExpectedQuat = FRotationMatrix::MakeFromXZ(FVector(1.f, 0.f, 0.f), Up).ToQuat();
	const FQuat ActualQuat = CamRoot->GetComponentQuat();

	TestTrue(TEXT("Camera root quat should match expected gravity-relative orientation"),
		ActualQuat.AngularDistance(ExpectedQuat) < FMath::DegreesToRadians(1.f));

	Char->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 6. Third-person spring arm matches camera quat
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentSpringArmMatchesQuat,
	"FederationGame.Planet.PlanetGravityComponent.ThirdPersonSpringArmMatchesQuat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentSpringArmMatchesQuat::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetGravityComponent* Comp = nullptr;
	ACharacter* Char = SpawnCharacterWithGravityComp(World, Comp);
	if (!Char || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	USceneComponent* CamRoot = NewObject<USceneComponent>(Char, TEXT("TestCamRoot"));
	CamRoot->SetupAttachment(Char->GetRootComponent());
	CamRoot->SetUsingAbsoluteRotation(true);
	CamRoot->RegisterComponent();

	USpringArmComponent* SpringArm = NewObject<USpringArmComponent>(Char, TEXT("TestSpringArm"));
	SpringArm->SetupAttachment(Char->GetRootComponent());
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->RegisterComponent();

	Comp->SetCameraReferences(CamRoot, SpringArm);
	Comp->GravityDir = FVector(0.f, 0.f, 1.f);
	Comp->bAlignToGravity = true;
	Comp->bUseGravityRelativeLook = true;
	Comp->ViewTangentForward = FVector(1.f, 0.f, 0.f);
	Comp->ViewPitchRad = 0.f;
	Comp->bViewInitialized = true;
	Comp->ViewUp = FVector(0.f, 0.f, -1.f);

	Comp->UpdateCameraOrientation();

	const FQuat CameraQuat = CamRoot->GetComponentQuat();
	const FQuat SpringArmQuat = SpringArm->GetComponentQuat();

	TestTrue(TEXT("Spring arm rotation should match camera root rotation"),
		CameraQuat.AngularDistance(SpringArmQuat) < FMath::DegreesToRadians(1.f));

	Char->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 7. RecoverGroundContact skips during ascent
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentRecoverSkipsDuringAscent,
	"FederationGame.Planet.PlanetGravityComponent.RecoverGroundContactSkipsDuringAscent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentRecoverSkipsDuringAscent::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetGravityComponent* Comp = nullptr;
	ACharacter* Char = SpawnCharacterWithGravityComp(World, Comp);
	if (!Char || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
	Comp->GravityDir = FVector(0.f, 0.f, 1.f);
	CMC->SetMovementMode(MOVE_Falling);
	CMC->Velocity = FVector(0.f, 0.f, -500.f); // ascending away from planet

	Comp->RecoverGroundContact();

	TestTrue(TEXT("Should remain in Falling mode during ascent"),
		CMC->MovementMode == MOVE_Falling);

	Char->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 8. ComponentTickUpdatesGravity
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentTickUpdatesGravity,
	"FederationGame.Planet.PlanetGravityComponent.ComponentTickUpdatesGravity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentTickUpdatesGravity::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetGravityComponent* Comp = nullptr;
	ACharacter* Char = SpawnCharacterWithGravityComp(World, Comp);
	if (!Char || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	AStaticMeshActor* Planet = World->SpawnActor<AStaticMeshActor>();
	Planet->Tags.Add(FName(TEXT("Planet")));
	Planet->SetActorLocation(FVector(1000.f, 0.f, 0.f));
	Planet->SetActorScale3D(FVector(10.f));

	Char->SetActorLocation(FVector::ZeroVector);

	// Manually call TickComponent to simulate a tick
	Comp->TickComponent(0.016f, LEVELTICK_All, nullptr);

	TestFalse(TEXT("Gravity direction should have been set after tick"),
		Comp->GravityDir.IsNearlyZero());
	TestTrue(TEXT("Gravity should point toward the planet (+X)"),
		FVector::DotProduct(Comp->GravityDir, FVector(1.f, 0.f, 0.f)) > 0.99f);

	Planet->Destroy();
	Char->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 9. Component aligns different actor types (non-Character AActor)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentAlignsDifferentActorTypes,
	"FederationGame.Planet.PlanetGravityComponent.AlignsDifferentActorTypes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentAlignsDifferentActorTypes::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	// Spawn a plain AActor (not ACharacter) with the component
	AActor* Actor = World->SpawnActor<AActor>();
	if (!Actor) { AddError(TEXT("Failed to spawn actor")); return false; }

	USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
	Root->RegisterComponent();
	Actor->SetRootComponent(Root);

	UPlanetGravityComponent* Comp = NewObject<UPlanetGravityComponent>(Actor, TEXT("TestGravityComp"));
	Comp->RegisterComponent();

	Comp->GravityDir = FVector(0.f, 1.f, 0.f);
	Comp->bAlignToGravity = true;

	for (int i = 0; i < 60; ++i)
	{
		Comp->UpdateGravityAlignment(0.05f);
	}

	const FVector ActorUp = Actor->GetActorUpVector();
	TestTrue(TEXT("Non-character actor up should oppose gravity direction"),
		FVector::DotProduct(ActorUp, FVector(0.f, -1.f, 0.f)) > 0.99f);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 10. GetGravityUp returns correct up vector
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetGravityComponentGetGravityUp,
	"FederationGame.Planet.PlanetGravityComponent.GetGravityUpReturnsCorrectVector",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetGravityComponentGetGravityUp::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetGravityComponent* Comp = nullptr;
	ACharacter* Char = SpawnCharacterWithGravityComp(World, Comp);
	if (!Char || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->GravityDir = FVector(0.f, 0.f, 1.f);
	TestTrue(TEXT("Gravity up should be -GravityDir"),
		FVector::DotProduct(Comp->GetGravityUp(), FVector(0.f, 0.f, -1.f)) > 0.99f);

	Comp->GravityDir = FVector::ZeroVector;
	TestTrue(TEXT("Gravity up should fall back to world up when no gravity"),
		FVector::DotProduct(Comp->GetGravityUp(), FVector::UpVector) > 0.99f);

	Char->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
