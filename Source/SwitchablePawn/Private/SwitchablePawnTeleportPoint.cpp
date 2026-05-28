#include "SwitchablePawnTeleportPoint.h"

#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SwitchableBaseCharacter.h"
#include "SwitchablePlayerController.h"
#include "UObject/UObjectGlobals.h"
#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

ASwitchablePawnTeleportPoint::ASwitchablePawnTeleportPoint()
{
	PrimaryActorTick.bCanEverTick = true;
#if WITH_EDITOR
	PrimaryActorTick.bStartWithTickEnabled = true;
#endif

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(SceneRoot);

	Gizmo = CreateDefaultSubobject<USphereComponent>(TEXT("Gizmo"));
	Gizmo->SetupAttachment(SceneRoot);
	Gizmo->SetSphereRadius(25.0f);
	Gizmo->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Gizmo->SetGenerateOverlapEvents(false);
	Gizmo->SetHiddenInGame(true);
	Gizmo->SetCanEverAffectNavigation(false);
}

void ASwitchablePawnTeleportPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		SyncActorLabelFromTeleportPointName();
		SyncTeleportPointNameFromActorLabel();
	}
#endif
}

void ASwitchablePawnTeleportPoint::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
bool ASwitchablePawnTeleportPoint::ShouldTickIfViewportsOnly() const
{
	return true;
}

void ASwitchablePawnTeleportPoint::PostRename(UObject* OldOuter, FName OldName)
{
	Super::PostRename(OldOuter, OldName);
	SyncTeleportPointNameFromActorLabel();
}
#endif

void ASwitchablePawnTeleportPoint::SyncTeleportPointNameFromActorLabel()
{
	if (bSyncingTeleportPointName)
	{
		return;
	}

	bSyncingTeleportPointName = true;
	TeleportPointName = FName(*GetActorLabel());
	bSyncingTeleportPointName = false;
}

void ASwitchablePawnTeleportPoint::SyncActorLabelFromTeleportPointName()
{
	if (bSyncingTeleportPointName || TeleportPointName.IsNone() || !GetWorld() || GetWorld()->IsGameWorld())
	{
		return;
	}

	bSyncingTeleportPointName = true;
	SetActorLabel(TeleportPointName.ToString(), false);
	bSyncingTeleportPointName = false;
}

void ASwitchablePawnTeleportPoint::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	(void)DeltaSeconds;

#if WITH_EDITOR
	if (!bSyncingTeleportPointName)
	{
		const FName ActorLabelName(*GetActorLabel());
		if (TeleportPointName != ActorLabelName)
		{
			SyncTeleportPointNameFromActorLabel();
		}
	}
#endif

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
