#include "SwitchablePawnStart.h"

#include "Components/ArrowComponent.h"

ASwitchablePawnStart::ASwitchablePawnStart()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(SceneRoot);
}

bool ASwitchablePawnStart::FindPresetPointByName(FName PointName, FSwitchableTeleportPoint& OutPoint) const
{
	for (const FSwitchableTeleportPoint& Point : PresetPoints)
	{
		if (Point.Name == PointName)
		{
			OutPoint = Point;
			return true;
		}
	}

	return false;
}

bool ASwitchablePawnStart::FindPresetPointByIndex(int32 Index, FSwitchableTeleportPoint& OutPoint) const
{
	if (!PresetPoints.IsValidIndex(Index))
	{
		return false;
	}

	OutPoint = PresetPoints[Index];
	return true;
}

FTransform ASwitchablePawnStart::GetStartTransform() const
{
	return GetActorTransform();
}

bool ASwitchablePawnStart::ResolvePresetPointTransform(const FSwitchableTeleportPoint& Point, FTransform& OutTransform) const
{
	if (Point.TargetActor)
	{
		OutTransform = Point.TargetActor->GetActorTransform();
		return true;
	}

	return false;
}
