#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwitchablePawnTypes.h"
#include "SwitchablePawnTeleportPoint.generated.h"

class UArrowComponent;
class USphereComponent;
struct FPropertyChangedEvent;

UCLASS(Blueprintable)
class SWITCHABLEPAWN_API ASwitchablePawnTeleportPoint : public AActor
{
	GENERATED_BODY()

public:
	ASwitchablePawnTeleportPoint();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostRename(UObject* OldOuter, FName OldName) override;
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport Point")
	FTransform GetTeleportTransform() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Teleport Point", meta = (ClampMin = 0.0))
	float ReturnDistanceThreshold = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Teleport Point")
	FName TeleportPointName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Teleport Point")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Teleport Point")
	TObjectPtr<UArrowComponent> Arrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Teleport Point")
	TObjectPtr<USphereComponent> Gizmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Teleport Point")
	bool bSetAsDefaultStart = false;

protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	void SyncActorLabelFromTeleportPointName();
	void SyncTeleportPointNameFromActorLabel();
	bool bSyncingTeleportPointName = false;
};
