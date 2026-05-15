#include "SwitchableVRCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionControllerComponent.h"
#include "NavigationSystem.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

ASwitchableVRCharacter::ASwitchableVRCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultTeleportPreviewMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (DefaultTeleportPreviewMesh.Succeeded())
	{
		TeleportPreviewMesh = DefaultTeleportPreviewMesh.Object;
	}

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

	if (bUseTeleportPreviewMesh)
	{
		RefreshTeleportPreviewMesh(Points);
	}
	else
	{
		ClearTeleportPreviewMesh();
	}
}

void ASwitchableVRCharacter::RefreshTeleportPreviewMesh(const TArray<FVector>& Points)
{
	if (!TeleportPreviewSpline)
	{
		return;
	}

	EnsureTeleportPreviewSegmentCount(FMath::Max(0, Points.Num() - 1));

	for (int32 SegmentIndex = 0; SegmentIndex < TeleportPreviewSegments.Num(); ++SegmentIndex)
	{
		USplineMeshComponent* Segment = TeleportPreviewSegments[SegmentIndex].Get();
		if (!Segment)
		{
			continue;
		}

		const bool bVisible = SegmentIndex < Points.Num() - 1;
		Segment->SetVisibility(bVisible, true);
		Segment->SetHiddenInGame(!bVisible);
		if (!bVisible)
		{
			continue;
		}

		const FVector StartPos = TeleportPreviewSpline->GetLocationAtSplinePoint(SegmentIndex, ESplineCoordinateSpace::Local);
		const FVector StartTangent = TeleportPreviewSpline->GetTangentAtSplinePoint(SegmentIndex, ESplineCoordinateSpace::Local);
		const FVector EndPos = TeleportPreviewSpline->GetLocationAtSplinePoint(SegmentIndex + 1, ESplineCoordinateSpace::Local);
		const FVector EndTangent = TeleportPreviewSpline->GetTangentAtSplinePoint(SegmentIndex + 1, ESplineCoordinateSpace::Local);

		Segment->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, true);
		Segment->SetStartScale(FVector2D(TeleportPreviewRadius, TeleportPreviewRadius));
		Segment->SetEndScale(FVector2D(TeleportPreviewRadius, TeleportPreviewRadius));

		if (TeleportPreviewMesh)
		{
			Segment->SetStaticMesh(TeleportPreviewMesh);
		}

		if (TeleportPreviewMaterial)
		{
			Segment->SetMaterial(0, TeleportPreviewMaterial);
		}
	}
}

void ASwitchableVRCharacter::EnsureTeleportPreviewSegmentCount(int32 SegmentCount)
{
	while (TeleportPreviewSegments.Num() < SegmentCount)
	{
		const int32 NewIndex = TeleportPreviewSegments.Num();
		USplineMeshComponent* Segment = NewObject<USplineMeshComponent>(this, *FString::Printf(TEXT("TeleportPreviewSegment_%d"), NewIndex));
		Segment->SetupAttachment(TeleportPreviewSpline);
		Segment->SetMobility(EComponentMobility::Movable);
		Segment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Segment->SetGenerateOverlapEvents(false);
		Segment->RegisterComponent();
		TeleportPreviewSegments.Add(Segment);
	}
}

void ASwitchableVRCharacter::ClearTeleportPreviewMesh()
{
	for (USplineMeshComponent* Segment : TeleportPreviewSegments)
	{
		if (Segment)
		{
			Segment->SetVisibility(false, true);
			Segment->SetHiddenInGame(true);
		}
	}
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
		ClearTeleportPreviewMesh();
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
				ClearTeleportPreviewMesh();
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
