#include "SwitchablePawnBlueprintLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "SwitchablePawnTeleportPoint.h"

FName USwitchablePawnBlueprintLibrary::MakeUniqueNameFromNames(FName BaseName, const TArray<FName>& ExistingNames)
{
	if (BaseName.IsNone())
	{
		return NAME_None;
	}

	auto IsTaken = [&ExistingNames](const FName& Candidate)
	{
		return ExistingNames.Contains(Candidate);
	};

	if (!IsTaken(BaseName))
	{
		return BaseName;
	}

	for (int32 Index = 0; Index < TNumericLimits<int32>::Max(); ++Index)
	{
		const FName Candidate(*FString::Printf(TEXT("%s_%d"), *BaseName.ToString(), Index));
		if (!IsTaken(Candidate))
		{
			return Candidate;
		}
	}

	return BaseName;
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
