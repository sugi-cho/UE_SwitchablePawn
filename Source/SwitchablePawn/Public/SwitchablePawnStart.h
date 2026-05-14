#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwitchablePawnTypes.h"
#include "SwitchablePawnStart.generated.h"

class UArrowComponent;

UCLASS(Blueprintable)
class SWITCHABLEPAWN_API ASwitchablePawnStart : public AActor
{
	GENERATED_BODY()

public:
	ASwitchablePawnStart();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Start")
	bool FindPresetPointByName(FName PointName, FSwitchableTeleportPoint& OutPoint) const;

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Start")
	bool FindPresetPointByIndex(int32 Index, FSwitchableTeleportPoint& OutPoint) const;

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Start")
	FTransform GetStartTransform() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Start")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Start")
	TObjectPtr<UArrowComponent> Arrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Start")
	FName StartName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Start")
	bool bUseAsDefaultStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Start")
	TArray<FSwitchableTeleportPoint> PresetPoints;
};
