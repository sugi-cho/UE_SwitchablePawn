#include "SwitchableVRCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionControllerComponent.h"
#include "NavigationSystem.h"
#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

ASwitchableVRCharacter::ASwitchableVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(RootComponent);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VRRoot);
	VRCamera->bUsePawnControlRotation = false;
	VRCamera->bLockToHmd = true;

	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(VRRoot);
	LeftController->MotionSource = TEXT("Left");

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(VRRoot);
	RightController->MotionSource = TEXT("Right");

	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftController);
	LeftHandMesh->SetRelativeLocation(FVector(-2.98126f, -3.5f, 4.561753f));
	LeftHandMesh->SetRelativeRotation(FRotator(90.0f, -25.0f, -180.0f));
	LeftHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftHandMesh->SetGenerateOverlapEvents(false);

	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightController);
	RightHandMesh->SetRelativeLocation(FVector(-2.98126f, 3.5f, 4.561753f));
	RightHandMesh->SetRelativeRotation(FRotator(89.99999f, 25.0f, 0.0f));
	RightHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightHandMesh->SetGenerateOverlapEvents(false);

	TeleportPreviewSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportPreviewSpline"));
	TeleportPreviewSpline->SetupAttachment(VRRoot);
	TeleportPreviewSpline->SetHiddenInGame(true);
	TeleportPreviewSpline->SetVisibility(true);
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

void ASwitchableVRCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshTeleportPreview();
}

#if WITH_EDITOR
void ASwitchableVRCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshTeleportPreview();
}
#endif

void ASwitchableVRCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bTeleportAiming)
	{
		UpdateTeleportAim();
	}

#if WITH_EDITOR
	if (GetWorld() && (GetWorld()->WorldType == EWorldType::Editor || GetWorld()->WorldType == EWorldType::EditorPreview))
	{
		RefreshTeleportPreview();
	}
#endif
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

void ASwitchableVRCharacter::BeginTeleportAim(bool bUseLeftHand)
{
	TeleportTraceController = bUseLeftHand ? LeftController : RightController;
	bTeleportAiming = true;
	UpdateTeleportAim();
}

FVector ASwitchableVRCharacter::GetTeleportTraceStartLocation(const UMotionControllerComponent* TraceController) const
{
	if (!TraceController)
	{
		return FVector::ZeroVector;
	}

	return TraceController->GetComponentTransform().TransformPosition(TeleportTraceStartOffset);
}

FRotator ASwitchableVRCharacter::GetTeleportTraceRotation(const UMotionControllerComponent* TraceController) const
{
	if (!TraceController)
	{
		return FRotator::ZeroRotator;
	}

	return (TraceController->GetComponentRotation() + TeleportTraceRotationOffset).GetNormalized();
}

void ASwitchableVRCharacter::RefreshTeleportPreview()
{
	if (!TeleportPreviewSpline || !GetWorld())
	{
		return;
	}

	const UMotionControllerComponent* TraceController = TeleportTraceController ? TeleportTraceController.Get() : RightController.Get();
	if (!TraceController)
	{
		return;
	}

	TArray<FVector> Points;
	const FVector Start = GetTeleportTraceStartLocation(TraceController);
	const FRotator Rotation = GetTeleportTraceRotation(TraceController);
	const FVector Forward = Rotation.Vector();
	const FVector Velocity = (Forward * TeleportArcInitialSpeed) + (GetActorUpVector() * (TeleportArcInitialSpeed * TeleportArcForwardBias));
	const FVector Gravity(0.0f, 0.0f, GetWorld()->GetGravityZ() * TeleportArcGravityScale);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SwitchableVRTeleportPreview), false, this);
	FVector CurrentPosition = Start;
	FVector CurrentVelocity = Velocity;

	Points.Add(Start);

	for (float Time = 0.0f; Time <= TeleportArcMaxTime; Time += TeleportArcTimeStep)
	{
		CurrentVelocity += Gravity * TeleportArcTimeStep;
		const FVector NextPosition = CurrentPosition + CurrentVelocity * TeleportArcTimeStep;
		FHitResult Hit;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CurrentPosition, NextPosition, ECC_Visibility, Params);
		Points.Add(bHit ? Hit.ImpactPoint : NextPosition);
		if (bHit)
		{
			break;
		}

		CurrentPosition = NextPosition;

		if (FVector::DistSquared(Start, CurrentPosition) > FMath::Square(TeleportTraceDistance))
		{
			break;
		}
	}

	TeleportPreviewSpline->ClearSplinePoints(false);
	for (int32 Index = 0; Index < Points.Num(); ++Index)
	{
		TeleportPreviewSpline->AddSplinePoint(Points[Index], ESplineCoordinateSpace::World, false);
	}
	TeleportPreviewSpline->UpdateSpline();
}

void ASwitchableVRCharacter::UpdateTeleportAim()
{
	bHasValidTeleportDestination = false;

	const UMotionControllerComponent* TraceController = TeleportTraceController ? TeleportTraceController.Get() : RightController.Get();
	if (!TraceController || !GetWorld())
	{
		return;
	}

	const FVector Start = GetTeleportTraceStartLocation(TraceController);
	const FVector Forward = GetTeleportTraceRotation(TraceController).Vector();
	const FVector Velocity = (Forward * TeleportArcInitialSpeed) + (GetActorUpVector() * (TeleportArcInitialSpeed * TeleportArcForwardBias));
	const FVector Gravity(0.0f, 0.0f, GetWorld()->GetGravityZ() * TeleportArcGravityScale);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SwitchableVRTeleport), false, this);
	FVector CurrentPosition = Start;
	FVector CurrentVelocity = Velocity;
	bool bHit = false;
	FHitResult Hit;

	for (float Time = 0.0f; Time <= TeleportArcMaxTime; Time += TeleportArcTimeStep)
	{
		CurrentVelocity += Gravity * TeleportArcTimeStep;
		const FVector NextPosition = CurrentPosition + CurrentVelocity * TeleportArcTimeStep;
		bHit = GetWorld()->LineTraceSingleByChannel(Hit, CurrentPosition, NextPosition, ECC_Visibility, Params);
		DrawDebugLine(GetWorld(), CurrentPosition, bHit ? Hit.ImpactPoint : NextPosition, bHit ? FColor::Green : FColor::Red, false, 0.0f, 0, 2.0f);

		if (bHit)
		{
			break;
		}

		CurrentPosition = NextPosition;

		if (FVector::DistSquared(Start, CurrentPosition) > FMath::Square(TeleportTraceDistance))
		{
			break;
		}
	}

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
	RefreshTeleportPreview();
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
	TeleportTraceController = nullptr;
	RefreshTeleportPreview();
}
