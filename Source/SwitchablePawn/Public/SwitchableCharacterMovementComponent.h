#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SwitchableCharacterMovementComponent.generated.h"

UCLASS()
class SWITCHABLEPAWN_API USwitchableCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	void SetNavMeshMovementEnabled(bool bEnabled);
};
