// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanetGravityComponent.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCapsuleComponent;
class UCharacterMovementComponent;

/**
 * Reusable component that provides radial planet gravity, capsule alignment,
 * gravity-relative camera orientation, and ground-contact recovery.
 *
 * Attach to any actor that needs to walk/stand on a spherical planet.
 * For player characters, pass camera references so the component can drive
 * first- and third-person view orientations via quaternion (no Euler round-trip).
 */
UCLASS(ClassGroup = "Federation", meta = (BlueprintSpawnableComponent))
class FEDERATION_API UPlanetGravityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlanetGravityComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Configuration ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gravity")
	bool bAlignToGravity = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gravity")
	float AlignInterpSpeed = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gravity|Camera")
	bool bUseGravityRelativeLook = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gravity|Camera")
	float MaxLookPitchDegrees = 85.f;

	// --- Optional camera references (set by owning actor) ---

	void SetCameraReferences(USceneComponent* InFirstPersonRoot, USpringArmComponent* InThirdPersonArm);

	// --- Public API ---

	UFUNCTION(BlueprintCallable, Category = "Gravity")
	FVector GetGravityDirection() const { return GravityDir; }

	UFUNCTION(BlueprintCallable, Category = "Gravity")
	FVector GetGravityUp() const { return GravityDir.IsNearlyZero() ? FVector::UpVector : (-GravityDir).GetSafeNormal(); }

	FVector GetViewTangentForward() const { return ViewTangentForward; }
	float GetViewPitchRad() const { return ViewPitchRad; }
	bool IsGravityViewInitialized() const { return bViewInitialized; }

	void ApplyLookInput(float YawDegrees, float PitchDegrees);

	void UpdatePlanetGravity();
	void UpdateGravityAlignment(float DeltaTime);
	void UpdateCameraOrientation();
	void RecoverGroundContact();
	void InitializeGravityRelativeView(const FVector& Up);

	// --- State (public for testing and direct access) ---

	FVector GravityDir;
	FVector ViewUp = FVector::UpVector;
	FVector ViewTangentForward = FVector::ForwardVector;
	bool bViewInitialized = false;
	float ViewPitchRad = 0.f;

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> FirstPersonCameraRoot;

	UPROPERTY()
	TObjectPtr<USpringArmComponent> ThirdPersonSpringArm;

	UCharacterMovementComponent* GetOwnerCMC() const;
	UCapsuleComponent* GetOwnerCapsule() const;
};
