// Copyright Federation Game. All Rights Reserved.

#include "Character/FederationCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	PrimaryActorTick.bCanEverTick = false;

	// Ensure the character mesh is visible to the owning player (see hands/arms/body in first person)
	GetMesh()->SetOwnerNoSee(false);
	GetMesh()->SetOnlyOwnerSee(false);

	// First-person camera: attach to mesh so it moves with the character; position at eye height
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Spring arm for third-person (optional)
	ThirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonSpringArm"));
	ThirdPersonSpringArm->SetupAttachment(GetCapsuleComponent());
	ThirdPersonSpringArm->TargetArmLength = 300.f;
	ThirdPersonSpringArm->bUsePawnControlRotation = true;
	ThirdPersonSpringArm->SetRelativeLocation(FVector(0.f, 0.f, 70.f));

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(ThirdPersonSpringArm);
	ThirdPersonCameraComponent->bUsePawnControlRotation = false;
}

void AFederationCharacter::BeginPlay()
{
	Super::BeginPlay();

	TryLoadDefaultMesh();
	SetupFirstPersonView();
	UpdateActiveCamera();
	SetupEnhancedInput();
}

void AFederationCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIC && MoveForwardAction && MoveRightAction && LookAction)
	{
		EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnMoveForward);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnMoveRight);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnLook);
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
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
	if (Controller)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Dir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Dir, Forward);
	}
}

void AFederationCharacter::OnMoveRight(const FInputActionValue& Value)
{
	const float Right = Value.Get<float>();
	if (Controller)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Dir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Dir, Right);
	}
}

void AFederationCharacter::OnLook(const FInputActionValue& Value)
{
	FVector2D Look = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(Look.X);
		AddControllerPitchInput(Look.Y);
	}
}

void AFederationCharacter::SetupEnhancedInput()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem) return;

	if (!DefaultMappingContext || !MoveForwardAction || !MoveRightAction || !LookAction)
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
	if (!LookAction)
	{
		LookAction = NewObject<UInputAction>(this, FName(TEXT("IA_Look_Default")));
		LookAction->ValueType = EInputActionValueType::Axis2D;
	}
	if (!JumpAction)
	{
		JumpAction = NewObject<UInputAction>(this, FName(TEXT("IA_Jump_Default")));
		JumpAction->ValueType = EInputActionValueType::Boolean;
	}

	if (DefaultMappingContext) return;

	UInputMappingContext* IMC = NewObject<UInputMappingContext>(this, FName(TEXT("IMC_Default")));
	IMC->MapKey(MoveForwardAction, EKeys::W);
	FEnhancedActionKeyMapping& S = IMC->MapKey(MoveForwardAction, EKeys::S);
	S.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	IMC->MapKey(MoveRightAction, EKeys::D);
	FEnhancedActionKeyMapping& A = IMC->MapKey(MoveRightAction, EKeys::A);
	A.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	IMC->MapKey(LookAction, EKeys::MouseX);
	IMC->MapKey(LookAction, EKeys::MouseY);
	IMC->MapKey(JumpAction, EKeys::SpaceBar);

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
	// If mesh has no "head" socket, attach camera to capsule at eye height
	if (!GetMesh()->DoesSocketExist(TEXT("head")))
	{
		FirstPersonCameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		FirstPersonCameraComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // Eye height above capsule center
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
