#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SwitchablePawnBlueprintLibrary.generated.h"

class ASwitchablePawnTeleportPoint;

UCLASS()
class SWITCHABLEPAWN_API USwitchablePawnBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Switchable Pawn|Name")
	static FName MakeUniqueNameFromNames(FName BaseName, const TArray<FName>& ExistingNames);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static ASwitchablePawnTeleportPoint* SpawnTeleportPoint(
		UObject* WorldContextObject,
		TSubclassOf<ASwitchablePawnTeleportPoint> TeleportPointClass,
		const FTransform& SpawnTransform,
		FName TeleportPointName,
		bool bSetAsDefaultStart = false);
};
