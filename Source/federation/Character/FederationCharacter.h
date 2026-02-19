// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FederationCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

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

	/** First-person camera (active when bUseThirdPersonView is false). */
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

	/** Toggle between first-person and third-person view. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleViewMode();

	/** Set view mode explicitly. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetThirdPersonView(bool bThirdPerson);

protected:
	virtual void BeginPlay() override;

	/** Apply first-person camera: attach to mesh at eye height, ensure body is visible to owner. */
	void SetupFirstPersonView();

	/** Apply third-person or first-person camera active state. */
	void UpdateActiveCamera();

	/** Try to load mannequin skeletal mesh from project (Animation Starter Pack or similar). */
	void TryLoadDefaultMesh();
};
