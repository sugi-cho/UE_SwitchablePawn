#include "SwitchablePawnTeleportPoint.h"

#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SwitchableBaseCharacter.h"
#include "SwitchablePlayerController.h"

ASwitchablePawnTeleportPoint::ASwitchablePawnTeleportPoint()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(SceneRoot);
}

void ASwitchablePawnTeleportPoint::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	(void)DeltaSeconds;

	if (!HasAuthority() || !bSetAsDefaultStart || ReturnDistanceThreshold <= 0.0f || !GetWorld())
	{
		return;
	}

const FVector StartLocation = GetActorLocation();
const FTransform StartTransform = GetTeleportTransform();
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

FTransform ASwitchablePawnTeleportPoint::GetTeleportTransform() const
{
	return GetActorTransform();
}
