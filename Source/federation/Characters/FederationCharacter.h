// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "FederationCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;

/**
 * Third/first-person playable character (FED-030).
 * Movement, camera, and Enhanced Input. Collision uses Pawn channel (capsule).
 * Input Actions can be set in Editor (Content/Input); if not set, defaults are created at runtime.
 */
UCLASS(Blueprintable, Category = "Federation")
class FEDERATION_API AFederationCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFederationCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Optional: assign Input Mapping Context from Content/Input. If null, a default IMC is created at runtime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Move forward/back (Axis1D). W = 1, S = -1. If null, created at runtime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveForwardAction;

	/** Move right/left (Axis1D). D = 1, A = -1. If null, created at runtime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveRightAction;

	/** Look (Axis2D). Mouse or gamepad. If null, created at runtime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	/** Jump. If null, created at runtime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	/** Spring arm for third-person camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnMoveForward(const FInputActionValue& Value);
	void OnMoveRight(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);

	/** Binds Enhanced Input: uses assigned assets or creates runtime defaults. */
	void SetupEnhancedInput();

	/** Creates default Input Actions and Mapping Context at runtime when none are assigned. */
	void CreateDefaultInputActionsAndContext();
};
