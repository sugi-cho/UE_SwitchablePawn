#pragma once

#include "CoreMinimal.h"
#include "SwitchableBaseCharacter.h"
#include "SwitchableFirstPersonCharacter.generated.h"

class UCameraComponent;

UCLASS(Blueprintable)
class SWITCHABLEPAWN_API ASwitchableFirstPersonCharacter : public ASwitchableBaseCharacter
{
	GENERATED_BODY()

public:
	ASwitchableFirstPersonCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Camera")
	float EyeHeight = 64.0f;
};
