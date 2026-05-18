#pragma once

#include "CoreMinimal.h"
#include "SwitchableBaseCharacter.h"
#include "SwitchableVRCharacter.generated.h"

class UCameraComponent;
class UMotionControllerComponent;
class USkeletalMesh;
class USkeletalMeshComponent;
class UMaterialInterface;
class UStaticMesh;
class USplineComponent;
class USplineMeshComponent;
struct FPropertyChangedEvent;

UCLASS(Blueprintable)
class SWITCHABLEPAWN_API ASwitchableVRCharacter : public ASwitchableBaseCharacter
{
	GENERATED_BODY()

public:
	ASwitchableVRCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Move(const FVector2D& MoveValue) override;
	virtual void Look(const FVector2D& LookValue) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|VR")
	void BeginTeleportAim(bool bUseLeftHand);

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
	bool bUseArcTeleportTrace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	FVector TeleportTraceStartOffset = FVector(-2.98126f, 0.0f, 24.561753f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	FRotator TeleportTraceRotationOffset = FRotator(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR", meta = (ClampMin = 0.0))
	float TeleportArcInitialSpeed = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR", meta = (ClampMin = 0.0))
	float TeleportArcGravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR", meta = (ClampMin = 0.001))
	float TeleportArcTimeStep = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR", meta = (ClampMin = 0.0))
	float TeleportArcMaxTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	float TeleportArcForwardBias = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	FVector TeleportProjectionExtent = FVector(100.0f, 100.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	bool bProjectTeleportToNavigation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR|Teleport Preview")
	bool bUseTeleportPreviewMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR|Teleport Preview")
	TObjectPtr<UStaticMesh> TeleportPreviewMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR|Teleport Preview")
	TObjectPtr<UMaterialInterface> TeleportPreviewMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR|Teleport Preview")
	TObjectPtr<UMaterialInterface> TeleportPreviewValidMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR|Teleport Preview", meta = (ClampMin = 0.001))
	float TeleportPreviewRadius = 0.025f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR|Auto Turn", meta = (ClampMin = 0.0))
	float TeleportAutoTurnStartDelay = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR|Auto Turn", meta = (ClampMin = 0.0))
	float TeleportAutoTurnStartAngle = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR|Auto Turn", meta = (ClampMin = 0.0))
	float TeleportAutoTurnStopAngle = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR|Auto Turn", meta = (ClampMin = 0.0))
	float TeleportAutoTurnSpeed = 120.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Switchable Pawn|VR")
	bool bHasValidTeleportDestination = false;

	UPROPERTY(BlueprintReadOnly, Category = "Switchable Pawn|VR")
	FVector TeleportDestination = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|VR")
	TObjectPtr<USplineComponent> TeleportPreviewSpline;

private:
	void UpdateVRRootOffset();
	void UpdateTeleportAutoTurn(float DeltaSeconds);
	float GetTeleportAimYawDelta() const;
	void RefreshTeleportPreview();
	void RefreshTeleportPreviewMesh(const TArray<FVector>& Points);
	void EnsureTeleportPreviewSegmentCount(int32 SegmentCount);
	void ClearTeleportPreviewMesh();
	FVector GetTeleportTraceStartLocation(const UMotionControllerComponent* TraceController) const;
	FRotator GetTeleportTraceRotation(const UMotionControllerComponent* TraceController) const;

	bool bTeleportAiming = false;
	float TeleportAutoTurnElapsed = 0.0f;
	TObjectPtr<UMotionControllerComponent> TeleportTraceController = nullptr;
	UPROPERTY(Transient)
	TArray<TObjectPtr<USplineMeshComponent>> TeleportPreviewSegments;
};
