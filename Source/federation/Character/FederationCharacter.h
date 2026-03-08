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
class UPlanetGravityComponent;
class UInventoryComponent;
class UJetpackMovementComponent;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RollAction;

	/** Jetpack boost (C): when jetpack on, toggles 50k speed + high acceleration. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JetpackBoostAction;

	/** Toggle dev diagnostics overlay (` / tilde). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleDevHUDAction;

	/** Toggle inventory panel (Tab). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	/** Toggle between first-person and third-person view. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleViewMode();

	/** Set view mode explicitly. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetThirdPersonView(bool bThirdPerson);

	/** Planet gravity component -- owns all radial-gravity, alignment, camera, and ground-recovery logic. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gravity")
	TObjectPtr<UPlanetGravityComponent> GravityComp;

	/** Inventory component -- manages carried items, equipment, and weight. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryComponent> InventoryComp;

	/** True when using flat gravity (e.g. on streamed surface level). When true, movement/look use control rotation and camera is driven from controller. */
	UFUNCTION(BlueprintCallable, Category = "Gravity")
	bool IsUsingFlatGravity() const;

	// --- Jetpack ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Jetpack")
	bool bJetpackActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Jetpack")
	TObjectPtr<UJetpackMovementComponent> JetpackComponent;

	UFUNCTION(BlueprintCallable, Category = "Movement|Jetpack")
	void ActivateJetpack();

	UFUNCTION(BlueprintCallable, Category = "Movement|Jetpack")
	void DeactivateJetpack();

	UFUNCTION(BlueprintCallable, Category = "Movement|Jetpack")
	bool IsJetpackEnabled() const;

	/** Syncs first- and third-person camera to the controller's control rotation. Used in flat mode each tick; exposed for tests. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void UpdateCameraForFlatMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;

	bool bJetpackThrustUp = false;

public:
	void OnJumpPressed();
	void OnJumpReleased();
	void OnJetpackBoostPressed();
	void OnToggleDevHUD();
	void OnToggleInventory();

protected:

	void SetupFirstPersonView();
	void InitializeSpaceViewFromCurrent();
	void ApplySpaceLookInput(float YawDegrees, float PitchDegrees, float RollDegrees = 0.f);
	void UpdateCameraForSpaceMode();

	void AddLookYaw(float Value);
	void AddLookPitch(float Value);

	void OnMoveForward(const FInputActionValue& Value);
	void OnMoveRight(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnLookYaw(const FInputActionValue& Value);
	void OnLookPitch(const FInputActionValue& Value);
	void OnRoll(const FInputActionValue& Value);

	void SetupEnhancedInput();
	void CreateDefaultInputActionsAndContext();

	void UpdateActiveCamera();

	void TryLoadDefaultMesh();
	void AddStarterItems();

	FQuat SpaceViewQuat = FQuat::Identity;
	bool bSpaceViewInitialized = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Space")
	float SpaceRollSpeedDegrees = 96.f;
};
