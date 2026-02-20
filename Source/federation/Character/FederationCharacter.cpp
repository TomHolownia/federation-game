// Copyright Federation Game. All Rights Reserved.

#include "Character/FederationCharacter.h"
#include "Planet/PlanetGravityComponent.h"
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
	ThirdPersonSpringArm->bUsePawnControlRotation = false;
	ThirdPersonSpringArm->bInheritRoll = false;
	ThirdPersonSpringArm->SetUsingAbsoluteRotation(true);
	ThirdPersonSpringArm->SetRelativeLocation(ThirdPersonSpringArmOffset);

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(ThirdPersonSpringArm);
	ThirdPersonCameraComponent->bUsePawnControlRotation = false;

	// Gravity component owns planet detection, capsule alignment, camera orientation, and ground recovery.
	GravityComp = CreateDefaultSubobject<UPlanetGravityComponent>(TEXT("PlanetGravity"));

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

	if (GravityComp)
	{
		GravityComp->SetCameraReferences(FirstPersonCameraRoot, ThirdPersonSpringArm);
	}
}

void AFederationCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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
	const FVector GravDir = GravityComp ? GravityComp->GetGravityDirection() : FVector::ZeroVector;
	if (GravityComp && GravityComp->bUseGravityRelativeLook && GravityComp->IsGravityViewInitialized() && !GravDir.IsNearlyZero())
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
	if (!Dir.IsNearlyZero()) AddMovementInput(Dir, Forward);
}

void AFederationCharacter::OnMoveRight(const FInputActionValue& Value)
{
	const float Right = Value.Get<float>();
	if (!Controller) return;

	FVector Dir;
	const FVector GravDir = GravityComp ? GravityComp->GetGravityDirection() : FVector::ZeroVector;
	if (GravityComp && GravityComp->bUseGravityRelativeLook && GravityComp->IsGravityViewInitialized() && !GravDir.IsNearlyZero())
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
	if (!Dir.IsNearlyZero()) AddMovementInput(Dir, Right);
}

void AFederationCharacter::OnLook(const FInputActionValue& Value)
{
	FVector2D Look = Value.Get<FVector2D>();
	if (!Controller) return;
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
	if (GravityComp && GravityComp->bUseGravityRelativeLook && GravityComp->bAlignToGravity && !GravityComp->GetGravityDirection().IsNearlyZero())
	{
		GravityComp->ApplyLookInput(0.f, Amount);
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
