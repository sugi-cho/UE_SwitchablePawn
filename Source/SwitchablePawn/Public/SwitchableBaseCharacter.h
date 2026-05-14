#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SwitchablePawnTypes.h"
#include "SwitchableBaseCharacter.generated.h"

UCLASS(Abstract, Blueprintable)
class SWITCHABLEPAWN_API ASwitchableBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASwitchableBaseCharacter();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Movement")
	virtual void Move(const FVector2D& MoveValue);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Movement")
	virtual void Look(const FVector2D& LookValue);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Movement")
	virtual void StartJump();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Movement")
	virtual void StopJump();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn")
	virtual void TeleportToSwitchableTransform(const FTransform& TargetTransform);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn")
	virtual FSwitchablePawnRuntimeState CaptureRuntimeState() const;

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn")
	virtual void ApplyRuntimeState(const FSwitchablePawnRuntimeState& RuntimeState);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn")
	virtual void SetSwitchableActive(bool bNewActive);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement")
	float WalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement")
	float JumpVelocity = 420.0f;
};
