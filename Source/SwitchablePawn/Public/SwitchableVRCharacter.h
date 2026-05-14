#pragma once

#include "CoreMinimal.h"
#include "SwitchableBaseCharacter.h"
#include "SwitchableVRCharacter.generated.h"

class UCameraComponent;
class UMotionControllerComponent;
class USkeletalMesh;
class USkeletalMeshComponent;

UCLASS(Blueprintable)
class SWITCHABLEPAWN_API ASwitchableVRCharacter : public ASwitchableBaseCharacter
{
	GENERATED_BODY()

public:
	ASwitchableVRCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Move(const FVector2D& MoveValue) override;
	virtual void Look(const FVector2D& LookValue) override;

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|VR")
	void BeginTeleportAim();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|VR")
	void UpdateTeleportAim();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|VR")
	bool ConfirmTeleport();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|VR")
	void CancelTeleportAim();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|VR")
	TObjectPtr<USceneComponent> VRRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|VR")
	TObjectPtr<UCameraComponent> VRCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|VR")
	TObjectPtr<UMotionControllerComponent> LeftController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|VR")
	TObjectPtr<UMotionControllerComponent> RightController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|VR")
	TObjectPtr<USkeletalMeshComponent> LeftHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|VR")
	TObjectPtr<USkeletalMeshComponent> RightHandMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	TObjectPtr<USkeletalMesh> HandSkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	float TeleportTraceDistance = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	FVector TeleportProjectionExtent = FVector(100.0f, 100.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	bool bProjectTeleportToNavigation = true;

	UPROPERTY(BlueprintReadOnly, Category = "Switchable Pawn|VR")
	bool bHasValidTeleportDestination = false;

	UPROPERTY(BlueprintReadOnly, Category = "Switchable Pawn|VR")
	FVector TeleportDestination = FVector::ZeroVector;

private:
	bool bTeleportAiming = false;
};
