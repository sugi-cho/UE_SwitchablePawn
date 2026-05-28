#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwitchablePawnTypes.h"
#include "SwitchablePawnTeleportPoint.generated.h"

class UArrowComponent;

UCLASS(Blueprintable)
class SWITCHABLEPAWN_API ASwitchablePawnTeleportPoint : public AActor
{
	GENERATED_BODY()

public:
	ASwitchablePawnTeleportPoint();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport Point")
	FTransform GetTeleportTransform() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Teleport Point", meta = (ClampMin = 0.0))
	float ReturnDistanceThreshold = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Teleport Point")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Teleport Point")
	TObjectPtr<UArrowComponent> Arrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Teleport Point")
	bool bSetAsDefaultStart = false;

protected:
	virtual void Tick(float DeltaSeconds) override;
};
