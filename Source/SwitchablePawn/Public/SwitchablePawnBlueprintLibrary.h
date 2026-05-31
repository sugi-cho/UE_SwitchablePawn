#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SwitchablePawnTypes.h"
#include "SwitchablePawnBlueprintLibrary.generated.h"

class ASwitchablePawnTeleportPoint;

UCLASS()
class SWITCHABLEPAWN_API USwitchablePawnBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Switchable Pawn|Enum")
	static bool StringToEnumValue(UEnum* Enum, const FString& InString, int64& OutValue);

	UFUNCTION(BlueprintPure, Category = "Switchable Pawn|Enum")
	static bool StringToSwitchablePawnMode(const FString& InString, ESwitchablePawnMode& OutMode);

	UFUNCTION(BlueprintPure, Category = "Switchable Pawn|Enum", meta = (DisplayName = "Enum Value To Name"))
	static FName EnumValueToName(ESwitchablePawnMode InValue);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static ASwitchablePawnTeleportPoint* SpawnTeleportPoint(
		UObject* WorldContextObject,
		TSubclassOf<ASwitchablePawnTeleportPoint> TeleportPointClass,
		const FTransform& SpawnTransform,
		FName TeleportPointName,
		bool bSetAsDefaultStart = false);
};
