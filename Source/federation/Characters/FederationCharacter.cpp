// Copyright Federation Game. All Rights Reserved.

#include "FederationCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"

AFederationCharacter::AFederationCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AFederationCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AFederationCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	SetupEnhancedInput();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC || !MoveForwardAction || !MoveRightAction || !LookAction) return;

	EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnMoveForward);
	EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnMoveRight);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFederationCharacter::OnLook);
	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
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
	// Forward: W = 1, S = -1
	IMC->MapKey(MoveForwardAction, EKeys::W);
	FEnhancedActionKeyMapping& S = IMC->MapKey(MoveForwardAction, EKeys::S);
	S.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	// Right: D = 1, A = -1
	IMC->MapKey(MoveRightAction, EKeys::D);
	FEnhancedActionKeyMapping& A = IMC->MapKey(MoveRightAction, EKeys::A);
	A.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	// Look: Mouse
	IMC->MapKey(LookAction, EKeys::MouseX);
	IMC->MapKey(LookAction, EKeys::MouseY);
	// Jump
	IMC->MapKey(JumpAction, EKeys::SpaceBar);

	DefaultMappingContext = IMC;
}
