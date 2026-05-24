#include "SwitchablePawnStart.h"

#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SwitchableBaseCharacter.h"
#include "SwitchablePlayerController.h"

ASwitchablePawnStart::ASwitchablePawnStart()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(SceneRoot);
}

void ASwitchablePawnStart::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	(void)DeltaSeconds;

	if (!HasAuthority() || !bUseAsDefaultStart || ReturnDistanceThreshold <= 0.0f || !GetWorld())
	{
		return;
	}

	const FVector StartLocation = GetActorLocation();
	const FTransform StartTransform = GetStartTransform();
	const float ReturnDistanceSq = FMath::Square(ReturnDistanceThreshold);

	for (int32 PlayerIndex = 0;; ++PlayerIndex)
	{
		ASwitchablePlayerController* SwitchableController = Cast<ASwitchablePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), PlayerIndex));
		if (!SwitchableController)
		{
			break;
		}

		ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(SwitchableController->GetPawn());
		if (!SwitchablePawn)
		{
			continue;
		}

		if (FVector::DistSquared(StartLocation, SwitchablePawn->GetActorLocation()) <= ReturnDistanceSq)
		{
			continue;
		}

		SwitchablePawn->TeleportToSwitchableTransform(StartTransform);
		if (UCharacterMovementComponent* Movement = SwitchablePawn->GetCharacterMovement())
		{
			Movement->Velocity = FVector::ZeroVector;
		}
		SwitchableController->SetControlRotation(StartTransform.Rotator());
	}
}

bool ASwitchablePawnStart::FindPresetPointByName(FName PointName, FSwitchableTeleportPoint& OutPoint) const
{
	for (const FSwitchableTeleportPoint& Point : PresetPoints)
	{
		if (Point.Name == PointName)
		{
			OutPoint = Point;
			return true;
		}
	}

	return false;
}

bool ASwitchablePawnStart::FindPresetPointByIndex(int32 Index, FSwitchableTeleportPoint& OutPoint) const
{
	if (!PresetPoints.IsValidIndex(Index))
	{
		return false;
	}

	OutPoint = PresetPoints[Index];
	return true;
}

FTransform ASwitchablePawnStart::GetStartTransform() const
{
	return GetActorTransform();
}

bool ASwitchablePawnStart::ResolvePresetPointTransform(const FSwitchableTeleportPoint& Point, FTransform& OutTransform) const
{
	if (Point.TargetActor)
	{
		OutTransform = Point.TargetActor->GetActorTransform();
		return true;
	}

	return false;
}
