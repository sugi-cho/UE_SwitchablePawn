#include "SwitchablePawnBlueprintLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/Class.h"
#include "SwitchablePawnTeleportPoint.h"

bool USwitchablePawnBlueprintLibrary::StringToEnumValue(UEnum* Enum, const FString& InString, int64& OutValue)
{
	if (!Enum)
	{
		return false;
	}

	const int64 Value = Enum->GetValueByNameString(InString, EGetByNameFlags::None);
	if (Value == INDEX_NONE)
	{
		return false;
	}

	OutValue = Value;
	return true;
}

bool USwitchablePawnBlueprintLibrary::StringToSwitchablePawnMode(const FString& InString, ESwitchablePawnMode& OutMode)
{
	const UEnum* Enum = StaticEnum<ESwitchablePawnMode>();
	if (!Enum)
	{
		return false;
	}

	const int64 Value = Enum->GetValueByNameString(InString, EGetByNameFlags::None);
	if (Value == INDEX_NONE)
	{
		return false;
	}

	OutMode = static_cast<ESwitchablePawnMode>(Value);
	return true;
}

ASwitchablePawnTeleportPoint* USwitchablePawnBlueprintLibrary::SpawnTeleportPoint(
	UObject* WorldContextObject,
	TSubclassOf<ASwitchablePawnTeleportPoint> TeleportPointClass,
	const FTransform& SpawnTransform,
	FName TeleportPointName,
	bool bSetAsDefaultStart)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World || !*TeleportPointClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = Cast<AActor>(WorldContextObject);

	ASwitchablePawnTeleportPoint* TeleportPoint = World->SpawnActorDeferred<ASwitchablePawnTeleportPoint>(
		TeleportPointClass,
		SpawnTransform,
		SpawnParams.Owner,
		nullptr,
		SpawnParams.SpawnCollisionHandlingOverride);
	if (!TeleportPoint)
	{
		return nullptr;
	}

	TeleportPoint->TeleportPointName = TeleportPointName;
	TeleportPoint->bSetAsDefaultStart = bSetAsDefaultStart;
	TeleportPoint->FinishSpawning(SpawnTransform);
	return TeleportPoint;
}
