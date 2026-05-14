#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SwitchablePawnTypes.h"
#include "SwitchablePlayerController.generated.h"

class ASwitchableBaseCharacter;
class ASwitchableFirstPersonCharacter;
class ASwitchablePawnStart;
class ASwitchableThirdPersonCharacter;
class ASwitchableVRCharacter;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS(Blueprintable)
class SWITCHABLEPAWN_API ASwitchablePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASwitchablePlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn")
	void SwitchToFirstPerson();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn")
	void SwitchToThirdPerson();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn")
	void SwitchToVR();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn")
	void SwitchMode(ESwitchablePawnMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport")
	bool TeleportToStartPoint(FName PointName);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport")
	bool TeleportToStartPointByIndex(int32 Index);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn")
	ESwitchablePawnMode DefaultMode = ESwitchablePawnMode::FirstPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn")
	TSubclassOf<ASwitchableFirstPersonCharacter> FirstPersonClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn")
	TSubclassOf<ASwitchableThirdPersonCharacter> ThirdPersonClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn")
	TSubclassOf<ASwitchableVRCharacter> VRClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn")
	bool bDestroyInactivePawns = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	int32 InputMappingPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputAction> MoveAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputAction> LookAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputAction> JumpAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputAction> SwitchFirstPersonAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputAction> SwitchThirdPersonAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputAction> SwitchVRAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputAction> VRTeleportAimAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputAction> VRTeleportConfirmAction = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Switchable Pawn")
	ESwitchablePawnMode CurrentMode = ESwitchablePawnMode::FirstPerson;

private:
	void EnsureFallbackInputAssets();
	void AddDefaultMappings();
	void AddMappingContextToLocalPlayer();
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleJumpStarted(const FInputActionValue& Value);
	void HandleJumpCompleted(const FInputActionValue& Value);
	void HandleVRTeleportAimStarted(const FInputActionValue& Value);
	void HandleVRTeleportAimCompleted(const FInputActionValue& Value);
	void HandleVRTeleportConfirm(const FInputActionValue& Value);
	bool CanEnterVRMode() const;
	void ApplyModeTransition(ESwitchablePawnMode NewMode, ESwitchablePawnMode PreviousMode);
	void SetWindowedMode();
	void SetVRModeEnabled(bool bEnabled);

	ASwitchableBaseCharacter* GetOrCreatePawnForMode(ESwitchablePawnMode Mode, const FSwitchablePawnRuntimeState& RuntimeState);
	TSubclassOf<ASwitchableBaseCharacter> GetPawnClassForMode(ESwitchablePawnMode Mode) const;
	ASwitchablePawnStart* FindDefaultSwitchableStart() const;
	bool FindTeleportPoint(FName PointName, FSwitchableTeleportPoint& OutPoint) const;
	bool FindTeleportPointByIndex(int32 Index, FSwitchableTeleportPoint& OutPoint) const;
	FSwitchablePawnRuntimeState BuildInitialRuntimeState() const;

	UPROPERTY(Transient)
	TObjectPtr<ASwitchableBaseCharacter> FirstPersonPawn = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ASwitchableBaseCharacter> ThirdPersonPawn = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ASwitchableBaseCharacter> VRPawn = nullptr;
};
