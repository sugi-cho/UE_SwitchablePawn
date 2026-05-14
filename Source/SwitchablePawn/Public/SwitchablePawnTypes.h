#pragma once

#include "CoreMinimal.h"
#include "SwitchablePawnTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class ESwitchablePawnMode : uint8
{
	FirstPerson,
	ThirdPerson,
	VR
};

USTRUCT(BlueprintType)
struct SWITCHABLEPAWN_API FSwitchableTeleportPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switchable Pawn")
	FName Name = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switchable Pawn")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switchable Pawn")
	bool bKeepCurrentYaw = false;
};

USTRUCT(BlueprintType)
struct SWITCHABLEPAWN_API FSwitchablePawnRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Switchable Pawn")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Switchable Pawn")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Switchable Pawn")
	FRotator ControlRotation = FRotator::ZeroRotator;
};
