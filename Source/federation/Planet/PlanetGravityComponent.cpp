// Copyright Federation Game. All Rights Reserved.

#include "Planet/PlanetGravityComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"

UPlanetGravityComponent::UPlanetGravityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlanetGravityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdatePlanetGravity();
	UpdateGravityAlignment(DeltaTime);
	UpdateCameraOrientation();
	RecoverGroundContact();
}

void UPlanetGravityComponent::SetCameraReferences(USceneComponent* InFirstPersonRoot, USpringArmComponent* InThirdPersonArm)
{
	FirstPersonCameraRoot = InFirstPersonRoot;
	ThirdPersonSpringArm = InThirdPersonArm;
}

UCharacterMovementComponent* UPlanetGravityComponent::GetOwnerCMC() const
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	return Char ? Char->GetCharacterMovement() : nullptr;
}

UCapsuleComponent* UPlanetGravityComponent::GetOwnerCapsule() const
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	return Char ? Char->GetCapsuleComponent() : nullptr;
}

// ---------------------------------------------------------------------------
// Gravity detection
// ---------------------------------------------------------------------------

void UPlanetGravityComponent::UpdatePlanetGravity()
{
	AActor* Owner = GetOwner();
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	UCharacterMovementComponent* CMC = GetOwnerCMC();
	if (!World || !Owner) return;

	TArray<AActor*> Planets;
	UGameplayStatics::GetAllActorsWithTag(World, FName(TEXT("Planet")), Planets);

	if (Planets.Num() == 0)
	{
		AActor* Largest = nullptr;
		float MaxScaleSq = 0.f;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AStaticMeshActor* SMA = Cast<AStaticMeshActor>(*It);
			if (!SMA || *It == Owner) continue;
			FVector S = SMA->GetActorScale3D();
			float ScaleSq = S.X * S.X + S.Y * S.Y + S.Z * S.Z;
			if (ScaleSq < 100.f) continue;
			float MaxS = FMath::Max3(S.X, S.Y, S.Z);
			float MinS = FMath::Min3(S.X, S.Y, S.Z);
			if (MaxS > 0.f && (MinS / MaxS) < 0.2f) continue;
			if (ScaleSq > MaxScaleSq)
			{
				MaxScaleSq = ScaleSq;
				Largest = *It;
			}
		}
		if (Largest) Planets.Add(Largest);
	}

	if (Planets.Num() == 0)
	{
		if (CMC) CMC->SetGravityDirection(FVector::DownVector);
		GravityDir = FVector::DownVector;
		return;
	}

	FVector MyLoc = Owner->GetActorLocation();
	AActor* Best = nullptr;
	float BestDistSq = FLT_MAX;
	for (AActor* P : Planets)
	{
		if (!P) continue;
		float DistSq = FVector::DistSquared(MyLoc, P->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = P;
		}
	}

	if (!Best)
	{
		if (CMC) CMC->SetGravityDirection(FVector::DownVector);
		GravityDir = FVector::DownVector;
		return;
	}

	FVector ToPlanet = Best->GetActorLocation() - MyLoc;
	float Len = ToPlanet.Size();
	if (Len < 1.f)
	{
		if (CMC) CMC->SetGravityDirection(FVector::DownVector);
		GravityDir = FVector::DownVector;
		return;
	}

	FVector Dir = ToPlanet / Len;
	if (CMC) CMC->SetGravityDirection(Dir);
	GravityDir = Dir;
}

// ---------------------------------------------------------------------------
// Capsule alignment
// ---------------------------------------------------------------------------

void UPlanetGravityComponent::UpdateGravityAlignment(float DeltaTime)
{
	if (!bAlignToGravity) return;
	if (GravityDir.IsNearlyZero()) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	const FVector DesiredUp = (-GravityDir).GetSafeNormal();

	FVector DesiredForward = Owner->GetActorForwardVector();
	if (bUseGravityRelativeLook && bViewInitialized)
	{
		const FVector TangentForward = (ViewTangentForward - (ViewTangentForward | DesiredUp) * DesiredUp).GetSafeNormal();
		if (!TangentForward.IsNearlyZero())
		{
			DesiredForward = TangentForward;
		}
	}
	else
	{
		DesiredForward = (DesiredForward - (DesiredForward | DesiredUp) * DesiredUp).GetSafeNormal();
		if (DesiredForward.IsNearlyZero())
		{
			DesiredForward = FVector::CrossProduct(DesiredUp, Owner->GetActorRightVector()).GetSafeNormal();
		}
	}

	const FQuat TargetQuat = FRotationMatrix::MakeFromXZ(DesiredForward, DesiredUp).ToQuat();
	const float AngleDiff = Owner->GetActorQuat().AngularDistance(TargetQuat);
	if (AngleDiff < FMath::DegreesToRadians(0.5f))
	{
		Owner->SetActorRotation(TargetQuat);
	}
	else
	{
		const FQuat NewQuat = FMath::QInterpTo(Owner->GetActorQuat(), TargetQuat, DeltaTime, AlignInterpSpeed);
		Owner->SetActorRotation(NewQuat);
	}
}

// ---------------------------------------------------------------------------
// Camera orientation (quaternion-driven, no Euler round-trip)
// ---------------------------------------------------------------------------

void UPlanetGravityComponent::UpdateCameraOrientation()
{
	if (!FirstPersonCameraRoot) return;

	const bool bGravityActive = bUseGravityRelativeLook && bAlignToGravity && !GravityDir.IsNearlyZero();
	if (!bGravityActive)
	{
		if (ThirdPersonSpringArm)
		{
			ThirdPersonSpringArm->bUsePawnControlRotation = true;
		}
		return;
	}

	if (ThirdPersonSpringArm)
	{
		ThirdPersonSpringArm->bUsePawnControlRotation = false;
	}

	const FVector Up = (-GravityDir).GetSafeNormal();

	if (!bViewInitialized || FVector::DotProduct(ViewUp.GetSafeNormal(), Up) < 0.999f)
	{
		InitializeGravityRelativeView(Up);
	}

	const float CosP = FMath::Cos(ViewPitchRad);
	const float SinP = FMath::Sin(ViewPitchRad);
	const FVector Forward = (ViewTangentForward * CosP + Up * SinP).GetSafeNormal();

	const FQuat ViewQuat = FRotationMatrix::MakeFromXZ(Forward, Up).ToQuat();
	FirstPersonCameraRoot->SetWorldRotation(ViewQuat);
	if (ThirdPersonSpringArm)
	{
		ThirdPersonSpringArm->SetWorldRotation(ViewQuat);
	}
}

// ---------------------------------------------------------------------------
// Gravity-relative view initialization
// ---------------------------------------------------------------------------

void UPlanetGravityComponent::InitializeGravityRelativeView(const FVector& Up)
{
	const FVector U = Up.GetSafeNormal();
	if (U.IsNearlyZero()) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Forward;
	if (bViewInitialized)
	{
		const float CP = FMath::Cos(ViewPitchRad);
		const float SP = FMath::Sin(ViewPitchRad);
		Forward = (ViewTangentForward * CP + ViewUp * SP).GetSafeNormal();
	}
	else
	{
		Forward = Owner->GetActorForwardVector();
	}

	const float UpComp = FMath::Clamp(FVector::DotProduct(Forward, U), -1.f, 1.f);
	ViewPitchRad = FMath::Asin(UpComp);

	FVector Tangent = (Forward - UpComp * U);
	if (!Tangent.Normalize())
	{
		Tangent = (Owner->GetActorForwardVector() - (Owner->GetActorForwardVector() | U) * U);
		if (!Tangent.Normalize())
		{
			Tangent = (ViewTangentForward - (ViewTangentForward | U) * U);
			Tangent.Normalize();
		}
	}
	if (!Tangent.IsNearlyZero())
	{
		ViewTangentForward = Tangent;
	}

	ViewUp = U;
	bViewInitialized = true;
}

// ---------------------------------------------------------------------------
// Look input (yaw/pitch relative to gravity)
// ---------------------------------------------------------------------------

void UPlanetGravityComponent::ApplyLookInput(float YawDegrees, float PitchDegrees)
{
	const FVector Up = (-GravityDir).GetSafeNormal();
	if (Up.IsNearlyZero()) return;

	if (!bViewInitialized || FVector::DotProduct(ViewUp.GetSafeNormal(), Up) < 0.999f)
	{
		InitializeGravityRelativeView(Up);
	}

	if (!FMath::IsNearlyZero(YawDegrees))
	{
		const float YawRad = FMath::DegreesToRadians(YawDegrees);
		ViewTangentForward = FQuat(Up, YawRad).RotateVector(ViewTangentForward);
		ViewTangentForward = (ViewTangentForward - (ViewTangentForward | Up) * Up);
		ViewTangentForward.Normalize();
	}

	if (!FMath::IsNearlyZero(PitchDegrees))
	{
		const float PitchRad = FMath::DegreesToRadians(PitchDegrees);
		const float MaxPitchRad = FMath::DegreesToRadians(MaxLookPitchDegrees);
		ViewPitchRad = FMath::Clamp(ViewPitchRad + PitchRad, -MaxPitchRad, MaxPitchRad);
	}

	ViewUp = Up;
}

// ---------------------------------------------------------------------------
// Ground contact recovery (curved-surface floor detection)
// ---------------------------------------------------------------------------

void UPlanetGravityComponent::RecoverGroundContact()
{
	UCharacterMovementComponent* CMC = GetOwnerCMC();
	if (!CMC || CMC->MovementMode != MOVE_Falling) return;
	if (GravityDir.IsNearlyZero()) return;

	const float VelAlongGravity = FVector::DotProduct(CMC->Velocity, GravityDir);
	if (VelAlongGravity < -10.f) return;

	UCapsuleComponent* Capsule = GetOwnerCapsule();
	if (!Capsule) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const FVector CapsuleBottom = Owner->GetActorLocation() + GravityDir * CapsuleHalfHeight;
	const float TraceLen = 15.f;
	const FVector End = CapsuleBottom + GravityDir * TraceLen;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GroundRecover), false, Owner);
	if (Owner->GetWorld()->LineTraceSingleByChannel(Hit, CapsuleBottom, End, ECC_Visibility, Params) && Hit.bBlockingHit)
	{
		CMC->SetMovementMode(MOVE_Walking);
	}
}
