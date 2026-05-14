#include "SwitchableVRCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionControllerComponent.h"
#include "NavigationSystem.h"

ASwitchableVRCharacter::ASwitchableVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(RootComponent);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VRRoot);
	VRCamera->bUsePawnControlRotation = false;

	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(VRRoot);
	LeftController->MotionSource = TEXT("Left");

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(VRRoot);
	RightController->MotionSource = TEXT("Right");

	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftController);

	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightController);
}

void ASwitchableVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HandSkeletalMesh)
	{
		LeftHandMesh->SetSkeletalMesh(HandSkeletalMesh);
		RightHandMesh->SetSkeletalMesh(HandSkeletalMesh);
	}
}

void ASwitchableVRCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bTeleportAiming)
	{
		UpdateTeleportAim();
	}
}

void ASwitchableVRCharacter::Move(const FVector2D& MoveValue)
{
	if (MoveValue.IsNearlyZero())
	{
		return;
	}

	const FRotator CameraYaw(0.0f, VRCamera ? VRCamera->GetComponentRotation().Yaw : GetActorRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MoveValue.Y);
	AddMovementInput(Right, MoveValue.X);
}

void ASwitchableVRCharacter::Look(const FVector2D& LookValue)
{
	if (!Controller || FMath::IsNearlyZero(LookValue.X))
	{
		return;
	}

	AddControllerYawInput(LookValue.X);
}

void ASwitchableVRCharacter::BeginTeleportAim()
{
	bTeleportAiming = true;
	UpdateTeleportAim();
}

void ASwitchableVRCharacter::UpdateTeleportAim()
{
	bHasValidTeleportDestination = false;

	if (!RightController || !GetWorld())
	{
		return;
	}

	const FVector Start = RightController->GetComponentLocation();
	const FVector End = Start + RightController->GetForwardVector() * TeleportTraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SwitchableVRTeleport), false, this);
	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	DrawDebugLine(GetWorld(), Start, bHit ? Hit.ImpactPoint : End, bHit ? FColor::Green : FColor::Red, false, 0.0f, 0, 2.0f);

	if (!bHit)
	{
		return;
	}

	FVector Candidate = Hit.ImpactPoint;
	if (bProjectTeleportToNavigation)
	{
		FNavLocation ProjectedLocation;
		if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			if (!NavSystem->ProjectPointToNavigation(Candidate, ProjectedLocation, TeleportProjectionExtent))
			{
				DrawDebugSphere(GetWorld(), Candidate, 20.0f, 12, FColor::Red, false, 0.0f);
				return;
			}
			Candidate = ProjectedLocation.Location;
		}
	}

	TeleportDestination = Candidate;
	bHasValidTeleportDestination = true;
	DrawDebugSphere(GetWorld(), TeleportDestination, 24.0f, 16, FColor::Green, false, 0.0f);
}

bool ASwitchableVRCharacter::ConfirmTeleport()
{
	if (!bHasValidTeleportDestination)
	{
		CancelTeleportAim();
		return false;
	}

	const float CapsuleHalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.0f;
	const FVector TargetLocation = TeleportDestination + FVector(0.0f, 0.0f, CapsuleHalfHeight);
	const FRotator TargetRotation(0.0f, VRCamera ? VRCamera->GetComponentRotation().Yaw : GetActorRotation().Yaw, 0.0f);

	const bool bTeleported = TeleportTo(TargetLocation, TargetRotation, false, true);
	CancelTeleportAim();
	return bTeleported;
}

void ASwitchableVRCharacter::CancelTeleportAim()
{
	bTeleportAiming = false;
	bHasValidTeleportDestination = false;
}
