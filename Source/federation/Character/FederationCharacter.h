// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "FederationCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USceneComponent;
class UInputMappingContext;
class UInputAction;

/**
 * First-person player character using the Animation Starter Pack mannequin.
 * Player can look down and see their hands/arms/body. Supports optional third-person view.
 */
UCLASS(Blueprintable, Category = "Federation")
class FEDERATION_API AFederationCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFederationCharacter(const FObjectInitializer& ObjectInitializer);

	/** Tuning offsets for different character meshes (can be adjusted per BP/asset). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Tuning")
	FVector MeshRelativeLocation = FVector(0.f, 0.f, -90.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Tuning")
	FRotator MeshRelativeRotation = FRotator(0.f, -90.f, 0.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning")
	FVector FirstPersonCameraRootOffset = FVector(0.f, 0.f, 64.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning")
	FVector FirstPersonCameraOffset = FVector(25.f, 0.f, 5.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning")
	FVector ThirdPersonSpringArmOffset = FVector(0.f, 0.f, 70.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning")
	float ThirdPersonArmLength = 300.f;

	/** First-person camera (active when bUseThirdPersonView is false). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USceneComponent> FirstPersonCameraRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/** Optional spring arm for third-person camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> ThirdPersonSpringArm;

	/** Third-person camera (active when bUseThirdPersonView is true). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> ThirdPersonCameraComponent;

	/** When true, use third-person camera; when false, use first-person. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bUseThirdPersonView = false;

	/** Optional: assign Input Mapping Context from Content/Input. If null, a default IMC is created at runtime (WASD, mouse look, jump). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveForwardAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveRightAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	/** Optional: split look input (yaw/pitch) when using runtime defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookYawAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookPitchAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ViewToggleAction;

	/** Toggle between first-person and third-person view. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleViewMode();

	/** Set view mode explicitly. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetThirdPersonView(bool bThirdPerson);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Update gravity to point toward nearest actor tagged "Planet" (radial planet gravity). */
	void UpdatePlanetGravity();

	/** Apply first-person camera: attach to mesh at eye height, ensure body is visible to owner. */
	void SetupFirstPersonView();

	/** Mouse look: add yaw/pitch to controller rotation (legacy axis fallback). */
	void AddLookYaw(float Value);
	void AddLookPitch(float Value);

	void OnMoveForward(const FInputActionValue& Value);
	void OnMoveRight(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnLookYaw(const FInputActionValue& Value);
	void OnLookPitch(const FInputActionValue& Value);

	/** Binds Enhanced Input: uses assigned assets or creates runtime defaults. */
	void SetupEnhancedInput();
	void CreateDefaultInputActionsAndContext();

	/** Apply third-person or first-person camera active state. */
	void UpdateActiveCamera();

	/** Try to load mannequin skeletal mesh from project (Animation Starter Pack or similar). */
	void TryLoadDefaultMesh();

	/** Cached gravity direction (toward planet) for surface-relative movement. */
	FVector LastGravityDir;
	FVector LastViewUp = FVector::UpVector;
	FVector LastViewTangentForward = FVector::ForwardVector;
	bool bGravityViewInitialized = false;
	float GravityViewPitchRad = 0.f;

	/** When true, rotate the capsule so actor up matches -gravity (lets CharacterMovement treat the sphere as "floor"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Tuning")
	bool bAlignCapsuleToGravity = true;

	/** How quickly to align capsule to gravity (higher = snappier). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Tuning")
	float GravityAlignInterpSpeed = 25.f;

	void UpdateGravityAlignment(float DeltaSeconds);

	/** When true, interpret look input relative to gravity (yaw about gravity-up, pitch about gravity-right). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning")
	bool bUseGravityRelativeLook = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Tuning")
	float MaxGravityLookPitchDegrees = 85.f;

	void UpdateCameraOrientation();
	void ApplyGravityRelativeLook(float YawDegrees, float PitchDegrees);
	void InitializeGravityRelativeView(const FVector& Up);
};
