// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JetpackMovementComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

/**
 * Reusable jetpack movement controller for character-like actors.
 * Owns enabled state and applies movement mode changes when toggled.
 */
UCLASS(ClassGroup = "Federation", meta = (BlueprintSpawnableComponent))
class FEDERATION_API UJetpackMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UJetpackMovementComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jetpack")
	float MaxJetpackSpeed = 10000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jetpack")
	bool bJetpackEnabled = false;

	/** When true (e.g. C key while jetpack on): 50k speed and high acceleration for testing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jetpack")
	bool bBoostEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jetpack|Boost")
	float BoostSpeed = 50000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jetpack|Boost")
	float BoostAcceleration = 200000.f;

	UFUNCTION(BlueprintCallable, Category = "Jetpack")
	void SetBoostEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Jetpack")
	bool IsBoostEnabled() const { return bBoostEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Jetpack")
	void ActivateJetpack();

	UFUNCTION(BlueprintCallable, Category = "Jetpack")
	void DeactivateJetpack();

	UFUNCTION(BlueprintCallable, Category = "Jetpack")
	void SetJetpackEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Jetpack")
	bool IsJetpackEnabled() const { return bJetpackEnabled; }

private:
	ACharacter* GetOwnerCharacter() const;
	UCharacterMovementComponent* GetOwnerCharacterMovement() const;
};
