// Copyright Federation Game. All Rights Reserved.

#include "Character/FederationCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"

AFederationCharacter::AFederationCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Ensure the character mesh is visible to the owning player (see hands/arms/body in first person)
	GetMesh()->SetOwnerNoSee(false);
	GetMesh()->SetOnlyOwnerSee(false);
	// Align mesh inside capsule (template-style defaults; can be tuned per character asset)
	GetMesh()->SetRelativeLocation(MeshRelativeLocation);
	GetMesh()->SetRelativeRotation(MeshRelativeRotation);

	// First-person camera: attach to capsule via a root so we can tune offsets per character asset.
	FirstPersonCameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FirstPersonCameraRoot"));
	FirstPersonCameraRoot->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraRoot->SetRelativeLocation(FirstPersonCameraRootOffset);
	// Important for custom gravity: don't inherit capsule rotation (we drive view via quaternion directly).
	FirstPersonCameraRoot->SetUsingAbsoluteRotation(true);

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonCameraRoot);
	FirstPersonCameraComponent->SetRelativeLocation(FirstPersonCameraOffset);
	FirstPersonCameraComponent->bUsePawnControlRotation = false;

	// Spring arm for third-person (optional)
	ThirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonSpringArm"));
	ThirdPersonSpringArm->SetupAttachment(GetCapsuleComponent());
	ThirdPersonSpringArm->TargetArmLength = ThirdPersonArmLength;
	ThirdPersonSpringArm->bUsePawnControlRotation = true;
	ThirdPersonSpringArm->bInheritRoll = false;
	ThirdPersonSpringArm->SetRelativeLocation(ThirdPersonSpringArmOffset);

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(ThirdPersonSpringArm);
	ThirdPersonCameraComponent->bUsePawnControlRotation = false;

	// Reduce sliding on slopes (e.g. planet surface)
	GetCharacterMovement()->GroundFriction = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	GetCharacterMovement()->BrakingFriction = 20.f;
	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
	// Slightly snappier movement/jump for iteration.
	GetCharacterMovement()->MaxWalkSpeed = 750.f;
	GetCharacterMovement()->JumpZVelocity = 520.f;
	// With custom (radial) gravity, accept any surface as walkable so we stand on the planet
	GetCharacterMovement()->SetWalkableFloorZ(0.f);
}

void AFederationCharacter::BeginPlay()
{
	Super::BeginPlay();

	TryLoadDefaultMesh();
	SetupFirstPersonView();
	UpdateActiveCamera();
	SetupEnhancedInput();
}

void AFederationCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdatePlanetGravity();
	UpdateGravityAlignment(DeltaSeconds);
	UpdateCameraOrientation();
}

void AFederationCharacter::UpdateCameraOrientation()
{
	if (!FirstPersonCameraRoot || !FirstPersonCameraComponent) return;

	const bool bGravityActive = bUseGravityRelativeLook && bAlignCapsuleToGravity && !LastGravityDir.IsNearlyZero();
	if (!bGravityActive)
	{
		FirstPersonCameraComponent->bUsePawnControlRotation = true;
		return;
	}

	FirstPersonCameraComponent->bUsePawnControlRotation = false;

	const FVector Up = (-LastGravityDir).GetSafeNormal();

	if (!bGravityViewInitialized || FVector::DotProduct(LastViewUp.GetSafeNormal(), Up) < 0.999f)
	{
		InitializeGravityRelativeView(Up);
	}

	const float CosP = FMath::Cos(GravityViewPitchRad);
	const float SinP = FMath::Sin(GravityViewPitchRad);
	const FVector Forward = (LastViewTangentForward * CosP + Up * SinP).GetSafeNormal();

	const FQuat ViewQuat = FRotationMatrix::MakeFromXZ(Forward, Up).ToQuat();
	FirstPersonCameraRoot->SetWorldRotation(ViewQuat);
}

void AFederationCharacter::InitializeGravityRelativeView(const FVector& Up)
{
	const FVector U = Up.GetSafeNormal();
	if (U.IsNearlyZero()) return;

	// Reconstruct current forward from existing state, or fall back to actor forward.
	FVector Forward;
	if (bGravityViewInitialized)
	{
		const float CP = FMath::Cos(GravityViewPitchRad);
		const float SP = FMath::Sin(GravityViewPitchRad);
		Forward = (LastViewTangentForward * CP + LastViewUp * SP).GetSafeNormal();
	}
	else
	{
		Forward = GetActorForwardVector();
	}

	// Decompose forward into new up-axis reference frame.
	const float UpComp = FMath::Clamp(FVector::DotProduct(Forward, U), -1.f, 1.f);
	GravityViewPitchRad = FMath::Asin(UpComp);

	FVector Tangent = (Forward - UpComp * U);
	if (!Tangent.Normalize())
	{
		Tangent = (GetActorForwardVector() - (GetActorForwardVector() | U) * U);
		if (!Tangent.Normalize())
		{
			Tangent = (LastViewTangentForward - (LastViewTangentForward | U) * U);
			Tangent.Normalize();
		}
	}
	if (!Tangent.IsNearlyZero())
	{
		LastViewTangentForward = Tangent;
	}

	LastViewUp = U;
	bGravityViewInitialized = true;
}

void AFederationCharacter::ApplyGravityRelativeLook(float YawDegrees, float PitchDegrees)
{
	const FVector Up = (-LastGravityDir).GetSafeNormal();
	if (Up.IsNearlyZero()) return;

	if (!bGravityViewInitialized || FVector::DotProduct(LastViewUp.GetSafeNormal(), Up) < 0.999f)
	{
		InitializeGravityRelativeView(Up);
	}

	// Yaw: rotate tangent-forward about gravity-up.
	if (!FMath::IsNearlyZero(YawDegrees))
	{
		const float YawRad = FMath::DegreesToRadians(YawDegrees);
		LastViewTangentForward = FQuat(Up, YawRad).RotateVector(LastViewTangentForward);
		LastViewTangentForward = (LastViewTangentForward - (LastViewTangentForward | Up) * Up);
		LastViewTangentForward.Normalize();
	}

	// Pitch: accumulate as a scalar angle (no Euler decomposition).
	if (!FMath::IsNearlyZero(PitchDegrees))
	{
		const float PitchRad = FMath::DegreesToRadians(PitchDegrees);
		const float MaxPitchRad = FMath::DegreesToRadians(MaxGravityLookPitchDegrees);
		GravityViewPitchRad = FMath::Clamp(GravityViewPitchRad + PitchRad, -MaxPitchRad, MaxPitchRad);
	}

	LastViewUp = Up;
}

void AFederationCharacter::UpdateGravityAlignment(float DeltaSeconds)
{
	if (!bAlignCapsuleToGravity) return;
	if (LastGravityDir.IsNearlyZero()) return;

	const FVector DesiredUp = (-LastGravityDir).GetSafeNormal();

	// Capsule body follows the view's tangent-forward (ignores pitch) so the character
	// faces the direction we're looking along the surface.
	FVector DesiredForward = GetActorForwardVector();
	if (bUseGravityRelativeLook && bGravityViewInitialized)
	{
		const FVector TangentForward = (LastViewTangentForward - (LastViewTangentForward | DesiredUp) * DesiredUp).GetSafeNormal();
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
			DesiredForward = FVector::CrossProduct(DesiredUp, GetActorRightVector()).GetSafeNormal();
		}
	}

	const FQuat TargetQuat = FRotationMatrix::MakeFromXZ(DesiredForward, DesiredUp).ToQuat();
	const FQuat NewQuat = FMath::QInterpTo(GetActorQuat(), TargetQuat, DeltaSeconds, GravityAlignInterpSpeed);
	SetActorRotation(NewQuat);
}

void AFederationCharacter::UpdatePlanetGravity()
{
	UWorld* World = GetWorld();
	if (!World || !GetCharacterMovement()) return;

	TArray<AActor*> Planets;
	UGameplayStatics::GetAllActorsWithTag(World, FName(TEXT("Planet")), Planets);
	// Fallback: if no tag, use largest planet-like StaticMeshActor (roughly uniform scale = sphere, not a flat floor)
	if (Planets.Num() == 0)
	{
		AActor* Largest = nullptr;
		float MaxScaleSq = 0.f;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AStaticMeshActor* SMA = Cast<AStaticMeshActor>(*It);
			if (!SMA || *It == this) continue;
			FVector S = SMA->GetActorScale3D();
			float ScaleSq = S.X * S.X + S.Y * S.Y + S.Z * S.Z;
			if (ScaleSq < 100.f) continue;
			// Require roughly uniform scale (sphere-like); reject flat floors (e.g. 50,50,0.1)
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
		GetCharacterMovement()->SetGravityDirection(FVector::DownVector);
		LastGravityDir = FVector::DownVector;
		return;
	}

	// Use closest planet as gravity source
	FVector MyLoc = GetActorLocation();
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
		GetCharacterMovement()->SetGravityDirection(FVector::DownVector);
		LastGravityDir = FVector::DownVector;
		return;
	}

	FVector ToPlanet = Best->GetActorLocation() - MyLoc;
	float Len = ToPlanet.Size();
	if (Len < 1.f)
	{
		GetCharacterMovement()->SetGravityDirection(FVector::DownVector);
		LastGravityDir = FVector::DownVector;
		return; // Inside or on center
	}

	// Gravity pulls toward planet center
	FVector GravityDir = ToPlanet / Len;
	GetCharacterMovement()->SetGravityDirection(GravityDir);
	LastGravityDir = GravityDir;
}

void AFederationCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Possession (and thus SetupPlayerInputComponent) happens before BeginPlay, so ensure our
	// runtime-created Enhanced Input actions + IMC exist before we attempt to bind them.
	if (!DefaultMappingContext || !MoveForwardAction || !MoveRightAction || !JumpAction || !ViewToggleAction ||
		(!LookAction && (!LookYawAction || !LookPitchAction)))
	{
		CreateDefaultInputActionsAndContext();
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIC && MoveForwardAction && MoveRightAction)
	{
		EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnMoveForward);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnMoveRight);
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnLook);
		}
		else if (LookYawAction && LookPitchAction)
		{
			EIC->BindAction(LookYawAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnLookYaw);
			EIC->BindAction(LookPitchAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnLookPitch);
		}
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (ViewToggleAction)
		{
			EIC->BindAction(ViewToggleAction, ETriggerEvent::Started, this, &AFederationCharacter::ToggleViewMode);
		}
	}
	else if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AFederationCharacter::AddLookYaw);
		PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AFederationCharacter::AddLookPitch);
	}
}

void AFederationCharacter::OnMoveForward(const FInputActionValue& Value)
{
	const float Forward = Value.Get<float>();
	if (!Controller) return;
	FVector Dir;
	if (bUseGravityRelativeLook && bGravityViewInitialized && !LastGravityDir.IsNearlyZero())
	{
		Dir = (LastViewTangentForward - (LastViewTangentForward | LastGravityDir) * LastGravityDir).GetSafeNormal();
		if (Dir.IsNearlyZero())
		{
			Dir = (GetActorForwardVector() - (GetActorForwardVector() | LastGravityDir) * LastGravityDir).GetSafeNormal();
		}
	}
	else if (LastGravityDir.IsNearlyZero())
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		Dir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	}
	else
	{
		Dir = (GetActorForwardVector() - (GetActorForwardVector() | LastGravityDir) * LastGravityDir).GetSafeNormal();
	}
	if (!Dir.IsNearlyZero()) AddMovementInput(Dir, Forward);
}

void AFederationCharacter::OnMoveRight(const FInputActionValue& Value)
{
	const float Right = Value.Get<float>();
	if (!Controller) return;
	FVector Dir;
	if (bUseGravityRelativeLook && bGravityViewInitialized && !LastGravityDir.IsNearlyZero())
	{
		const FVector GravityUp = (-LastGravityDir).GetSafeNormal();
		Dir = FVector::CrossProduct(GravityUp, LastViewTangentForward).GetSafeNormal();
		if (Dir.IsNearlyZero())
		{
			Dir = (GetActorRightVector() - (GetActorRightVector() | LastGravityDir) * LastGravityDir).GetSafeNormal();
		}
	}
	else if (LastGravityDir.IsNearlyZero())
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		Dir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}
	else
	{
		Dir = (GetActorRightVector() - (GetActorRightVector() | LastGravityDir) * LastGravityDir).GetSafeNormal();
	}
	if (!Dir.IsNearlyZero()) AddMovementInput(Dir, Right);
}

void AFederationCharacter::OnLook(const FInputActionValue& Value)
{
	FVector2D Look = Value.Get<FVector2D>();
	if (!Controller) return;
	if (bUseGravityRelativeLook && bAlignCapsuleToGravity && !LastGravityDir.IsNearlyZero())
	{
		ApplyGravityRelativeLook(Look.X, Look.Y);
		return;
	}
	AddControllerYawInput(Look.X);
	AddControllerPitchInput(Look.Y);
}

void AFederationCharacter::OnLookYaw(const FInputActionValue& Value)
{
	const float Amount = Value.Get<float>();
	if (!Controller) return;
	if (bUseGravityRelativeLook && bAlignCapsuleToGravity && !LastGravityDir.IsNearlyZero())
	{
		ApplyGravityRelativeLook(Amount, 0.f);
		return;
	}
	AddControllerYawInput(Amount);
}

void AFederationCharacter::OnLookPitch(const FInputActionValue& Value)
{
	const float Amount = Value.Get<float>();
	if (!Controller) return;
	if (bUseGravityRelativeLook && bAlignCapsuleToGravity && !LastGravityDir.IsNearlyZero())
	{
		ApplyGravityRelativeLook(0.f, Amount);
		return;
	}
	AddControllerPitchInput(Amount);
}

void AFederationCharacter::SetupEnhancedInput()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem) return;

	if (!DefaultMappingContext || !MoveForwardAction || !MoveRightAction || !JumpAction ||
		(!LookAction && (!LookYawAction || !LookPitchAction)))
	{
		CreateDefaultInputActionsAndContext();
	}

	if (DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AFederationCharacter::CreateDefaultInputActionsAndContext()
{
	if (!MoveForwardAction)
	{
		MoveForwardAction = NewObject<UInputAction>(this, FName(TEXT("IA_MoveForward_Default")));
		MoveForwardAction->ValueType = EInputActionValueType::Axis1D;
	}
	if (!MoveRightAction)
	{
		MoveRightAction = NewObject<UInputAction>(this, FName(TEXT("IA_MoveRight_Default")));
		MoveRightAction->ValueType = EInputActionValueType::Axis1D;
	}
	// Runtime defaults: split look into two 1D actions to avoid needing swizzle modifiers.
	if (!LookYawAction)
	{
		LookYawAction = NewObject<UInputAction>(this, FName(TEXT("IA_LookYaw_Default")));
		LookYawAction->ValueType = EInputActionValueType::Axis1D;
	}
	if (!LookPitchAction)
	{
		LookPitchAction = NewObject<UInputAction>(this, FName(TEXT("IA_LookPitch_Default")));
		LookPitchAction->ValueType = EInputActionValueType::Axis1D;
	}
	if (!JumpAction)
	{
		JumpAction = NewObject<UInputAction>(this, FName(TEXT("IA_Jump_Default")));
		JumpAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!ViewToggleAction)
	{
		ViewToggleAction = NewObject<UInputAction>(this, FName(TEXT("IA_ViewToggle_Default")));
		ViewToggleAction->ValueType = EInputActionValueType::Boolean;
	}

	if (DefaultMappingContext) return;

	UInputMappingContext* IMC = NewObject<UInputMappingContext>(this, FName(TEXT("IMC_Default")));
	IMC->MapKey(MoveForwardAction, EKeys::W);
	FEnhancedActionKeyMapping& S = IMC->MapKey(MoveForwardAction, EKeys::S);
	S.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	IMC->MapKey(MoveRightAction, EKeys::D);
	FEnhancedActionKeyMapping& A = IMC->MapKey(MoveRightAction, EKeys::A);
	A.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	IMC->MapKey(LookYawAction, EKeys::MouseX);
	// Note: For gravity-relative look, pitch sign is handled in code; don't double-invert here.
	IMC->MapKey(LookPitchAction, EKeys::MouseY);
	IMC->MapKey(JumpAction, EKeys::SpaceBar);
	IMC->MapKey(ViewToggleAction, EKeys::V);

	DefaultMappingContext = IMC;
}

void AFederationCharacter::AddLookYaw(float Value)
{
	AddControllerYawInput(Value);
}

void AFederationCharacter::AddLookPitch(float Value)
{
	AddControllerPitchInput(Value);
}

void AFederationCharacter::SetupFirstPersonView()
{
	// Camera is attached to capsule via FirstPersonCameraRoot; offsets are tunable via UPROPERTY.
	if (FirstPersonCameraRoot)
	{
		FirstPersonCameraRoot->SetRelativeLocation(FirstPersonCameraRootOffset);
	}
	if (FirstPersonCameraComponent)
	{
		FirstPersonCameraComponent->SetRelativeLocation(FirstPersonCameraOffset);
	}
}

void AFederationCharacter::UpdateActiveCamera()
{
	const bool bThird = bUseThirdPersonView;
	FirstPersonCameraComponent->SetActive(!bThird);
	ThirdPersonCameraComponent->SetActive(bThird);
	if (ThirdPersonSpringArm)
	{
		ThirdPersonSpringArm->SetActive(bThird);
	}
}

void AFederationCharacter::ToggleViewMode()
{
	bUseThirdPersonView = !bUseThirdPersonView;
	UpdateActiveCamera();
}

void AFederationCharacter::SetThirdPersonView(bool bThirdPerson)
{
	bUseThirdPersonView = bThirdPerson;
	UpdateActiveCamera();
}

void AFederationCharacter::TryLoadDefaultMesh()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || MeshComp->GetSkeletalMeshAsset())
	{
		return; // Already has a mesh (e.g. set in Blueprint)
	}

	// Same fallback order as PlaceActorsFromDataCommand for consistency
	const TCHAR* Paths[] = {
		TEXT("/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin"),
		TEXT("/Game/AnimationStarterPack/Character/Mesh/UE4_Mannequin.UE4_Mannequin"),
		TEXT("/Game/AnimationStarterPack/Character/Mesh/SKM_Mannequin.SKM_Mannequin"),
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"),
		TEXT("/Engine/EngineMeshes/SkeletalMesh/DefaultCharacter.DefaultCharacter"),
	};
	for (const TCHAR* Path : Paths)
	{
		if (USkeletalMesh* Loaded = LoadObject<USkeletalMesh>(nullptr, Path))
		{
			MeshComp->SetSkeletalMesh(Loaded);
			break;
		}
	}
}
