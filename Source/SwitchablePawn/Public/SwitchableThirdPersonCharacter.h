#pragma once

#include "CoreMinimal.h"
#include "SwitchableBaseCharacter.h"
#include "SwitchableThirdPersonCharacter.generated.h"

class UCameraComponent;
class USkeletalMesh;
class USpringArmComponent;

UCLASS(Blueprintable)
class SWITCHABLEPAWN_API ASwitchableThirdPersonCharacter : public ASwitchableBaseCharacter
{
	GENERATED_BODY()

public:
	ASwitchableThirdPersonCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switchable Pawn|Camera")
	TObjectPtr<UCameraComponent> ThirdPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Model")
	TObjectPtr<USkeletalMesh> BodyMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Camera")
	float CameraDistance = 350.0f;
};
