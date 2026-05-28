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
	SyncActorNameFromTeleportPointName();
}

void ASwitchablePawnTeleportPoint::BeginPlay()
{
	Super::BeginPlay();
	SyncActorNameFromTeleportPointName();
}

#if WITH_EDITOR
void ASwitchablePawnTeleportPoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ASwitchablePawnTeleportPoint, TeleportPointName))
	{
		SyncActorNameFromTeleportPointName();
	}
}

void ASwitchablePawnTeleportPoint::PostRename(UObject* OldOuter, FName OldName)
{
	Super::PostRename(OldOuter, OldName);
	SyncTeleportPointNameFromActorName();
}
#endif

void ASwitchablePawnTeleportPoint::SyncActorNameFromTeleportPointName()
{
	if (bSyncingTeleportPointName || TeleportPointName.IsNone() || !GetWorld() || TeleportPointName == GetFName())
	{
		return;
	}

	bSyncingTeleportPointName = true;
	const FName UniqueName = MakeUniqueObjectName(GetOuter(), GetClass(), TeleportPointName);
	if (GetFName() != UniqueName)
	{
		Rename(*UniqueName.ToString(), GetOuter());
#if WITH_EDITOR
		SetActorLabel(UniqueName.ToString(), false);
#endif
	}

	TeleportPointName = UniqueName;
	bSyncingTeleportPointName = false;
}

void ASwitchablePawnTeleportPoint::SyncTeleportPointNameFromActorName()
{
	if (bSyncingTeleportPointName)
	{
		return;
	}

	bSyncingTeleportPointName = true;
	TeleportPointName = GetFName();
#if WITH_EDITOR
	SetActorLabel(TeleportPointName.ToString(), false);
#endif
	bSyncingTeleportPointName = false;
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
