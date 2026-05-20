#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SwitchablePawnTypes.h"
#include "SwitchableBaseCharacter.generated.h"

class ANavLinkProxy;

UCLASS(Abstract, Blueprintable)
class SWITCHABLEPAWN_API ASwitchableBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASwitchableBaseCharacter(const FObjectInitializer& ObjectInitializer);

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

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Movement")
	virtual void SetConstrainMovementToNavMesh(bool bNewConstrain);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Movement")
	virtual void SetAffectedByGravity(bool bNewAffectedByGravity);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement")
	float WalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement")
	float JumpVelocity = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement")
	bool bConstrainMovementToNavMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement")
	bool bAffectedByGravity = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement|NavLink")
	bool bUseNavLinkProxyTraversal = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement|NavLink", meta = (ClampMin = 0.0))
	float NavLinkActivationRadius = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement|NavLink")
	TArray<TObjectPtr<ANavLinkProxy>> NavLinkProxies;

protected:
	void RefreshMovementModeForNavigation();
	bool TryTraverseNavLinkProxy(const FVector& MoveDirection);
};
