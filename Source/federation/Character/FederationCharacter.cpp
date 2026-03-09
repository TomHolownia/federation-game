// Copyright Federation Game. All Rights Reserved.

#include "Character/FederationCharacter.h"
#include "Core/FederationGameState.h"
#include "Movement/JetpackMovementComponent.h"
#include "Planet/PlanetGravityComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemBase.h"
#include "Inventory/WeaponItem.h"
#include "Inventory/EquipmentItem.h"
#include "Inventory/ConsumableItem.h"
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
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Core/FederationHUD.h"
#include "GameFramework/InputSettings.h"

AFederationCharacter::AFederationCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Hide character mesh from the owning player so it doesn't block the first-person view.
	// FED-039 set this to false for see-your-body; revisit when animations are wired up (FED-040).
	GetMesh()->SetOwnerNoSee(true);
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
	ThirdPersonSpringArm->bUsePawnControlRotation = false;
	ThirdPersonSpringArm->bInheritRoll = false;
	ThirdPersonSpringArm->SetUsingAbsoluteRotation(true);
	ThirdPersonSpringArm->SetRelativeLocation(ThirdPersonSpringArmOffset);

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(ThirdPersonSpringArm);
	ThirdPersonCameraComponent->bUsePawnControlRotation = false;

	// Gravity component owns planet detection, capsule alignment, camera orientation, and ground recovery.
	GravityComp = CreateDefaultSubobject<UPlanetGravityComponent>(TEXT("PlanetGravity"));
	JetpackComponent = CreateDefaultSubobject<UJetpackMovementComponent>(TEXT("JetpackMovement"));

	InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

	// GroundFriction only applies in Walking mode; FallingLateralFriction (default 0) applies in air.
	// Do NOT use bUseSeparateBrakingFriction — it overrides falling friction too and kills air velocity.
	GetCharacterMovement()->GroundFriction = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	// Slightly snappier movement/jump for iteration.
	GetCharacterMovement()->MaxWalkSpeed = 750.f;
	GetCharacterMovement()->JumpZVelocity = 520.f;
	GetCharacterMovement()->AirControl = 0.f;
	// With custom (radial) gravity, accept any surface as walkable so we stand on the planet
	GetCharacterMovement()->SetWalkableFloorZ(0.f);
	GetCharacterMovement()->MaxStepHeight = 55.f;
	GetCharacterMovement()->PerchRadiusThreshold = 30.f;
	GetCharacterMovement()->PerchAdditionalHeight = 50.f;
}

void AFederationCharacter::BeginPlay()
{
	Super::BeginPlay();

	TryLoadDefaultMesh();
	SetupFirstPersonView();
	UpdateActiveCamera();
	SetupEnhancedInput();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			// Space mode wants unconstrained look (no hard +/-89 pitch clamp).
			PC->PlayerCameraManager->ViewPitchMin = -179.9f;
			PC->PlayerCameraManager->ViewPitchMax = 179.9f;
		}
	}

	if (GravityComp)
	{
		GravityComp->SetCameraReferences(FirstPersonCameraRoot, ThirdPersonSpringArm);
	}

	AddStarterItems();
}

bool AFederationCharacter::IsUsingFlatGravity() const
{
	if (!GravityComp || !GravityComp->IsComponentTickEnabled())
	{
		return true;
	}
	return GravityComp->GetSurfaceBlendAlpha() >= 0.5f;
}

bool AFederationCharacter::IsJetpackEnabled() const
{
	return JetpackComponent && JetpackComponent->IsJetpackEnabled();
}

void AFederationCharacter::UpdateCameraForFlatMode()
{
	if (!Controller) return;
	const FRotator ControlRot = Controller->GetControlRotation();
	if (FirstPersonCameraRoot)
	{
		FirstPersonCameraRoot->SetWorldRotation(ControlRot);
	}
	if (ThirdPersonSpringArm)
	{
		ThirdPersonSpringArm->SetWorldRotation(ControlRot);
	}
}

void AFederationCharacter::InitializeSpaceViewFromCurrent()
{
	if (FirstPersonCameraRoot)
	{
		SpaceViewQuat = FirstPersonCameraRoot->GetComponentQuat();
	}
	else if (Controller)
	{
		SpaceViewQuat = Controller->GetControlRotation().Quaternion();
	}
	else
	{
		SpaceViewQuat = GetActorQuat();
	}
	SpaceViewQuat.Normalize();
	bSpaceViewInitialized = true;
}

void AFederationCharacter::ApplySpaceLookInput(float YawDegrees, float PitchDegrees, float RollDegrees)
{
	if (!bSpaceViewInitialized)
	{
		InitializeSpaceViewFromCurrent();
	}

	if (!FMath::IsNearlyZero(YawDegrees))
	{
		const FVector UpAxis = SpaceViewQuat.GetUpVector().GetSafeNormal();
		SpaceViewQuat = FQuat(UpAxis, FMath::DegreesToRadians(YawDegrees)) * SpaceViewQuat;
	}
	if (!FMath::IsNearlyZero(PitchDegrees))
	{
		const FVector RightAxis = SpaceViewQuat.GetRightVector().GetSafeNormal();
		SpaceViewQuat = FQuat(RightAxis, FMath::DegreesToRadians(PitchDegrees)) * SpaceViewQuat;
	}
	if (!FMath::IsNearlyZero(RollDegrees))
	{
		const FVector ForwardAxis = SpaceViewQuat.GetForwardVector().GetSafeNormal();
		SpaceViewQuat = FQuat(ForwardAxis, FMath::DegreesToRadians(RollDegrees)) * SpaceViewQuat;
	}
	SpaceViewQuat.Normalize();
}

void AFederationCharacter::UpdateCameraForSpaceMode()
{
	if (!bSpaceViewInitialized)
	{
		InitializeSpaceViewFromCurrent();
	}

	if (FirstPersonCameraRoot)
	{
		FirstPersonCameraRoot->SetWorldRotation(SpaceViewQuat);
	}
	if (ThirdPersonSpringArm)
	{
		ThirdPersonSpringArm->bUsePawnControlRotation = false;
		ThirdPersonSpringArm->SetWorldRotation(SpaceViewQuat);
	}
	if (Controller)
	{
		Controller->SetControlRotation(SpaceViewQuat.Rotator());
	}
}

void AFederationCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bIsFalling = GetCharacterMovement() && GetCharacterMovement()->MovementMode == MOVE_Falling;

	if (IsJetpackEnabled() || (bIsFalling && !IsUsingFlatGravity()))
	{
		UpdateCameraForSpaceMode();
	}
	else if (IsUsingFlatGravity())
	{
		UpdateCameraForFlatMode();
		bSpaceViewInitialized = false;
	}

	bJetpackActive = IsJetpackEnabled();

	if (IsPlayerControlled())
	{
		if (AFederationGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFederationGameState>() : nullptr)
		{
			GS->DebugJetpackEnabled = bJetpackActive;
			GS->DebugJetpackBoost = bJetpackActive && JetpackComponent && JetpackComponent->IsBoostEnabled();
		}
	}
}


void AFederationCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Possession (and thus SetupPlayerInputComponent) happens before BeginPlay, so ensure our
	// runtime-created Enhanced Input actions + IMC exist before we attempt to bind them.
	if (!DefaultMappingContext || !MoveForwardAction || !MoveRightAction || !JumpAction || !ViewToggleAction || !RollAction ||
		!ToggleDevHUDAction || !ToggleInventoryAction ||
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
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AFederationCharacter::OnJumpPressed);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &AFederationCharacter::OnJumpReleased);
		}
		if (ViewToggleAction)
		{
			EIC->BindAction(ViewToggleAction, ETriggerEvent::Started, this, &AFederationCharacter::ToggleViewMode);
		}
		if (RollAction)
		{
			EIC->BindAction(RollAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnRoll);
		}
		if (JetpackBoostAction)
		{
			EIC->BindAction(JetpackBoostAction, ETriggerEvent::Started, this, &AFederationCharacter::OnJetpackBoostPressed);
		}
		if (ToggleDevHUDAction)
		{
			EIC->BindAction(ToggleDevHUDAction, ETriggerEvent::Started, this, &AFederationCharacter::OnToggleDevHUD);
		}
		if (ToggleInventoryAction)
		{
			EIC->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AFederationCharacter::OnToggleInventory);
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
	if (IsJetpackEnabled())
	{
		Dir = FirstPersonCameraRoot ? FirstPersonCameraRoot->GetForwardVector() : Controller->GetControlRotation().Vector();
	}
	else if (IsUsingFlatGravity())
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		Dir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	}
	else
	{
		const FVector GravDir = GravityComp->GetGravityDirection();
		if (GravityComp->bUseGravityRelativeLook && GravityComp->IsGravityViewInitialized() && !GravDir.IsNearlyZero())
		{
			const FVector TangentFwd = GravityComp->GetViewTangentForward();
			Dir = (TangentFwd - (TangentFwd | GravDir) * GravDir).GetSafeNormal();
			if (Dir.IsNearlyZero())
			{
				Dir = (GetActorForwardVector() - (GetActorForwardVector() | GravDir) * GravDir).GetSafeNormal();
			}
		}
		else if (GravDir.IsNearlyZero())
		{
			const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
			Dir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		}
		else
		{
			Dir = (GetActorForwardVector() - (GetActorForwardVector() | GravDir) * GravDir).GetSafeNormal();
		}
	}
	if (!Dir.IsNearlyZero()) AddMovementInput(Dir, Forward);
}

void AFederationCharacter::OnMoveRight(const FInputActionValue& Value)
{
	const float Right = Value.Get<float>();
	if (!Controller) return;

	FVector Dir;
	if (IsJetpackEnabled())
	{
		Dir = FirstPersonCameraRoot ? FirstPersonCameraRoot->GetRightVector() : FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::Y);
	}
	else if (IsUsingFlatGravity())
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		Dir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}
	else
	{
		const FVector GravDir = GravityComp->GetGravityDirection();
		if (GravityComp->bUseGravityRelativeLook && GravityComp->IsGravityViewInitialized() && !GravDir.IsNearlyZero())
		{
			const FVector GravUp = GravityComp->GetGravityUp();
			Dir = FVector::CrossProduct(GravUp, GravityComp->GetViewTangentForward()).GetSafeNormal();
			if (Dir.IsNearlyZero())
			{
				Dir = (GetActorRightVector() - (GetActorRightVector() | GravDir) * GravDir).GetSafeNormal();
			}
		}
		else if (GravDir.IsNearlyZero())
		{
			const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
			Dir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		}
		else
		{
			Dir = (GetActorRightVector() - (GetActorRightVector() | GravDir) * GravDir).GetSafeNormal();
		}
	}
	if (!Dir.IsNearlyZero()) AddMovementInput(Dir, Right);
}

void AFederationCharacter::OnLook(const FInputActionValue& Value)
{
	FVector2D Look = Value.Get<FVector2D>();
	if (!Controller) return;
	// Jetpack in space: camera is driven from controller, so route look to controller
	if (GetCharacterMovement() && GetCharacterMovement()->MovementMode == MOVE_Flying)
	{
		ApplySpaceLookInput(Look.X, -Look.Y);
		return;
	}
	if (GetCharacterMovement() && GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		if (!IsUsingFlatGravity())
		{
			ApplySpaceLookInput(Look.X, -Look.Y);
			return;
		}
	}
	if (IsUsingFlatGravity())
	{
		AddControllerYawInput(Look.X);
		AddControllerPitchInput(-Look.Y);  // Invert pitch so mouse up = look up on surface
		return;
	}
	if (GravityComp && GravityComp->bUseGravityRelativeLook && GravityComp->bAlignToGravity && !GravityComp->GetGravityDirection().IsNearlyZero())
	{
		GravityComp->ApplyLookInput(Look.X, Look.Y);
		return;
	}
	AddControllerYawInput(Look.X);
	AddControllerPitchInput(Look.Y);
}

void AFederationCharacter::OnLookYaw(const FInputActionValue& Value)
{
	const float Amount = Value.Get<float>();
	if (!Controller) return;
	if (GetCharacterMovement() && GetCharacterMovement()->MovementMode == MOVE_Flying)
	{
		ApplySpaceLookInput(Amount, 0.f);
		return;
	}
	if (GetCharacterMovement() && GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		if (!IsUsingFlatGravity())
		{
			ApplySpaceLookInput(Amount, 0.f);
			return;
		}
	}
	if (IsUsingFlatGravity())
	{
		AddControllerYawInput(Amount);
		return;
	}
	if (GravityComp && GravityComp->bUseGravityRelativeLook && GravityComp->bAlignToGravity && !GravityComp->GetGravityDirection().IsNearlyZero())
	{
		GravityComp->ApplyLookInput(Amount, 0.f);
		return;
	}
	AddControllerYawInput(Amount);
}

void AFederationCharacter::OnLookPitch(const FInputActionValue& Value)
{
	const float Amount = Value.Get<float>();
	if (!Controller) return;
	if (GetCharacterMovement() && GetCharacterMovement()->MovementMode == MOVE_Flying)
	{
		ApplySpaceLookInput(0.f, -Amount);
		return;
	}
	if (GetCharacterMovement() && GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		if (!IsUsingFlatGravity())
		{
			ApplySpaceLookInput(0.f, -Amount);
			return;
		}
	}
	if (IsUsingFlatGravity())
	{
		AddControllerPitchInput(-Amount);  // Invert pitch so mouse up = look up on surface
		return;
	}
	if (GravityComp && GravityComp->bUseGravityRelativeLook && GravityComp->bAlignToGravity && !GravityComp->GetGravityDirection().IsNearlyZero())
	{
		GravityComp->ApplyLookInput(0.f, Amount);
		return;
	}
	AddControllerPitchInput(Amount);
}

void AFederationCharacter::OnRoll(const FInputActionValue& Value)
{
	const float Amount = Value.Get<float>();
	if (!Controller) return;

	const bool bSpaceLike = IsJetpackEnabled()
		|| ((GetCharacterMovement() && GetCharacterMovement()->MovementMode == MOVE_Falling) && !IsUsingFlatGravity());
	if (!bSpaceLike) return;

	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
	ApplySpaceLookInput(0.f, 0.f, Amount * SpaceRollSpeedDegrees * DeltaSeconds);
}

void AFederationCharacter::SetupEnhancedInput()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem) return;

	if (!DefaultMappingContext || !MoveForwardAction || !MoveRightAction || !JumpAction || !RollAction ||
		!ToggleDevHUDAction || !ToggleInventoryAction ||
		(!LookAction && (!LookYawAction || !LookPitchAction)))
	{
		CreateDefaultInputActionsAndContext();
	}

	if (DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Free the tilde key from the console so it can drive the dev HUD toggle
	if (UInputSettings* InputSettings = UInputSettings::GetInputSettings())
	{
		InputSettings->ConsoleKeys.Remove(EKeys::Tilde);
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
	if (!RollAction)
	{
		RollAction = NewObject<UInputAction>(this, FName(TEXT("IA_Roll_Default")));
		RollAction->ValueType = EInputActionValueType::Axis1D;
	}
	if (!JetpackBoostAction)
	{
		JetpackBoostAction = NewObject<UInputAction>(this, FName(TEXT("IA_JetpackBoost_Default")));
		JetpackBoostAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!ToggleDevHUDAction)
	{
		ToggleDevHUDAction = NewObject<UInputAction>(this, FName(TEXT("IA_ToggleDevHUD_Default")));
		ToggleDevHUDAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!ToggleInventoryAction)
	{
		ToggleInventoryAction = NewObject<UInputAction>(this, FName(TEXT("IA_ToggleInventory_Default")));
		ToggleInventoryAction->ValueType = EInputActionValueType::Boolean;
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
	IMC->MapKey(RollAction, EKeys::Q);
	FEnhancedActionKeyMapping& E = IMC->MapKey(RollAction, EKeys::E);
	E.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	IMC->MapKey(JetpackBoostAction, EKeys::C);
	IMC->MapKey(ToggleDevHUDAction, EKeys::Tilde);
	IMC->MapKey(ToggleInventoryAction, EKeys::Tab);

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

void AFederationCharacter::OnJumpPressed()
{
	if (IsJetpackEnabled())
	{
		DeactivateJetpack();
		return;
	}

	if (GetCharacterMovement() && !GetCharacterMovement()->IsMovingOnGround())
	{
		ActivateJetpack();
		return;
	}

	Jump();
}

void AFederationCharacter::OnJumpReleased()
{
	StopJumping();
	bJetpackThrustUp = false;
}

void AFederationCharacter::OnJetpackBoostPressed()
{
	if (JetpackComponent && IsJetpackEnabled())
	{
		JetpackComponent->SetBoostEnabled(!JetpackComponent->IsBoostEnabled());
	}
}

void AFederationCharacter::ActivateJetpack()
{
	if (JetpackComponent)
	{
		JetpackComponent->ActivateJetpack();
	}
	InitializeSpaceViewFromCurrent();
	bJetpackActive = IsJetpackEnabled();
}

void AFederationCharacter::DeactivateJetpack()
{
	bJetpackThrustUp = false;
	if (JetpackComponent)
	{
		JetpackComponent->DeactivateJetpack();
	}
	bSpaceViewInitialized = false;
	bJetpackActive = IsJetpackEnabled();
}

void AFederationCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	DeactivateJetpack();
}

void AFederationCharacter::OnToggleDevHUD()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	AFederationHUD* HUD = Cast<AFederationHUD>(PC->GetHUD());
	if (HUD) HUD->ToggleDevDiagnostics();
}

void AFederationCharacter::OnToggleInventory()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	AFederationHUD* HUD = Cast<AFederationHUD>(PC->GetHUD());
	if (HUD) HUD->ToggleInventory();
}

void AFederationCharacter::AddStarterItems()
{
	if (!InventoryComp || InventoryComp->GetItems().Num() > 0) return;

	auto MakeEquipment = [](FName ID, const FString& Name, EEquipmentSlot InSlot, float Weight)
	{
		UEquipmentItem* E = NewObject<UEquipmentItem>();
		E->ItemID = ID;
		E->DisplayName = FText::FromString(Name);
		E->Slot = InSlot;
		E->Weight = Weight;
		return E;
	};

	auto MakeWeapon = [](FName ID, const FString& Name, EWeaponType Type, EWeaponClass Class, float Weight)
	{
		UWeaponItem* W = NewObject<UWeaponItem>();
		W->ItemID = ID;
		W->DisplayName = FText::FromString(Name);
		W->WeaponType = Type;
		W->WeaponClass = Class;
		W->Weight = Weight;
		return W;
	};

	InventoryComp->AddItem(MakeEquipment(FName("BasicHelmet"), TEXT("Basic Helmet"), EEquipmentSlot::Head, 2.0f));
	InventoryComp->AddItem(MakeEquipment(FName("BasicVest"), TEXT("Basic Vest"), EEquipmentSlot::Body, 1.5f));
	InventoryComp->AddItem(MakeEquipment(FName("BasicBoots"), TEXT("Basic Boots"), EEquipmentSlot::Shoes, 1.0f));
	InventoryComp->AddItem(MakeEquipment(FName("BasicShield"), TEXT("Basic Shield"), EEquipmentSlot::Shield, 3.0f));

	UEquipmentItem* Adrenaline = MakeEquipment(FName("AdrenalineSurge"), TEXT("Adrenaline Surge"), EEquipmentSlot::Ability1, 0.5f);
	Adrenaline->Category = EItemCategory::Ability;
	InventoryComp->AddItem(Adrenaline);

	UEquipmentItem* Plating = MakeEquipment(FName("DermalPlating"), TEXT("Dermal Plating"), EEquipmentSlot::Biomorph1, 1.5f);
	Plating->Category = EItemCategory::Biomorph;
	InventoryComp->AddItem(Plating);

	InventoryComp->AddItem(MakeWeapon(FName("EnergyPistol"), TEXT("Energy Pistol"), EWeaponType::Energy, EWeaponClass::Pistol, 2.5f));
	InventoryComp->AddItem(MakeWeapon(FName("CombatKnife"), TEXT("Combat Knife"), EWeaponType::Kinetic, EWeaponClass::MeleeBladed, 1.0f));

	UConsumableItem* MedPack = NewObject<UConsumableItem>();
	MedPack->ItemID = FName("MedPack");
	MedPack->DisplayName = FText::FromString(TEXT("Med Pack"));
	MedPack->ConsumableType = EConsumableType::Medical;
	MedPack->Weight = 0.5f;
	MedPack->MaxStackSize = 5;
	InventoryComp->AddItem(MedPack, 3);

	UItemBase* AmmoCell = NewObject<UItemBase>();
	AmmoCell->ItemID = FName("AmmoCell");
	AmmoCell->DisplayName = FText::FromString(TEXT("Ammo Cell"));
	AmmoCell->Category = EItemCategory::Ammunition;
	AmmoCell->Weight = 0.2f;
	AmmoCell->MaxStackSize = 20;
	InventoryComp->AddItem(AmmoCell, 10);

	UConsumableItem* Rations = NewObject<UConsumableItem>();
	Rations->ItemID = FName("RationPack");
	Rations->DisplayName = FText::FromString(TEXT("Ration Pack"));
	Rations->ConsumableType = EConsumableType::Food;
	Rations->Weight = 0.3f;
	Rations->MaxStackSize = 10;
	InventoryComp->AddItem(Rations, 5);
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
