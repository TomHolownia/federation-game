// Copyright Federation Game. All Rights Reserved.

#include "Movement/JetpackMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UJetpackMovementComponent::UJetpackMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ACharacter* UJetpackMovementComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

UCharacterMovementComponent* UJetpackMovementComponent::GetOwnerCharacterMovement() const
{
	ACharacter* Character = GetOwnerCharacter();
	return Character ? Character->GetCharacterMovement() : nullptr;
}

void UJetpackMovementComponent::ActivateJetpack()
{
	SetJetpackEnabled(true);
}

void UJetpackMovementComponent::DeactivateJetpack()
{
	SetJetpackEnabled(false);
}

void UJetpackMovementComponent::SetBoostEnabled(bool bEnabled)
{
	if (bBoostEnabled == bEnabled) return;
	bBoostEnabled = bEnabled;

	UCharacterMovementComponent* CMC = GetOwnerCharacterMovement();
	if (!CMC || !bJetpackEnabled || CMC->MovementMode != MOVE_Flying) return;

	CMC->MaxFlySpeed = bBoostEnabled ? BoostSpeed : MaxJetpackSpeed;
	if (bBoostEnabled)
	{
		CMC->MaxAcceleration = BoostAcceleration;
		CMC->BrakingDecelerationFlying = 0.f;
	}
	else
	{
		CMC->MaxAcceleration = 1024.f;
		CMC->BrakingDecelerationFlying = 2048.f;
	}
}

void UJetpackMovementComponent::SetJetpackEnabled(bool bEnabled)
{
	if (bJetpackEnabled == bEnabled)
	{
		return;
	}

	bJetpackEnabled = bEnabled;
	UE_LOG(LogTemp, Log, TEXT("Jetpack Enabled: %s"), bJetpackEnabled ? TEXT("True") : TEXT("False"));

	UCharacterMovementComponent* CMC = GetOwnerCharacterMovement();
	if (!CMC)
	{
		return;
	}

	if (bJetpackEnabled)
	{
		CMC->SetMovementMode(MOVE_Flying);
		const float Speed = bBoostEnabled ? BoostSpeed : MaxJetpackSpeed;
		const float Accel = bBoostEnabled ? BoostAcceleration : 0.f; // 0 = use CMC default
		CMC->MaxFlySpeed = Speed;
		if (Accel > 0.f)
		{
			CMC->MaxAcceleration = Accel;
			CMC->BrakingDecelerationFlying = 0.f;
		}
	}
	else if (CMC->MovementMode == MOVE_Flying)
	{
		CMC->SetMovementMode(MOVE_Falling);
	}
}
