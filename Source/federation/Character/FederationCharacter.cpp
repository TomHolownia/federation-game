// Copyright Federation Game. All Rights Reserved.

#include "Character/FederationCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMesh.h"

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
