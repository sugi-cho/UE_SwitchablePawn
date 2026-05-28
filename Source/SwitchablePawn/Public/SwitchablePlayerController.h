#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "SwitchablePawnTypes.h"
#include "SwitchablePlayerController.generated.h"

class ASwitchableBaseCharacter;
class ASwitchableFirstPersonCharacter;
class ASwitchablePawnTeleportPoint;
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

	UFUNCTION(BlueprintImplementableEvent, Category = "Switchable Pawn")
	void OnModeWillChange(ESwitchablePawnMode PreviousMode, ESwitchablePawnMode NewMode, APawn* PreviousPawn, ASwitchableBaseCharacter* NewPawn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Switchable Pawn")
	void OnModeChanged(ESwitchablePawnMode PreviousMode, ESwitchablePawnMode NewMode, APawn* PreviousPawn, ASwitchableBaseCharacter* NewPawn);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Movement")
	void SetConstrainMovementToNavMesh(bool bNewConstrain);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Movement")
	void SetAffectedByGravity(bool bNewAffectedByGravity);

	UFUNCTION(BlueprintPure, Category = "Switchable Pawn|Movement")
	bool IsAffectedByGravity() const { return bAffectedByGravity; }

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|VR")
	void SetVRTeleportMovementEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintPure, Category = "Switchable Pawn|VR")
	bool IsVRTeleportMovementEnabled() const { return bEnableVRTeleportMovement; }

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Input")
	void SetMovementInputEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintPure, Category = "Switchable Pawn|Input")
	bool IsMovementInputEnabled() const { return bEnableMovementInput; }

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Input")
	void SetLookInputEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintPure, Category = "Switchable Pawn|Input")
	bool IsLookInputEnabled() const { return bEnableLookInput; }

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport")
	bool TeleportToStartPoint();

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport")
	bool TeleportToPointByName(FName PointName);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport")
	bool TeleportToPoint(ASwitchablePawnTeleportPoint* TeleportPoint);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport")
	bool TeleportToPointByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Switchable Pawn|Teleport")
	ASwitchablePawnTeleportPoint* SpawnTeleportPoint(const FTransform& SpawnTransform, FName TeleportPointName, bool bSetAsDefaultStart = false);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement")
	bool bConstrainMovementToNavMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Movement")
	bool bAffectedByGravity = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|VR")
	bool bEnableVRTeleportMovement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	int32 InputMappingPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	bool bEnableModeSwitchKeys = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	bool bEnableMovementInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	bool bEnableLookInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input", meta = (EditCondition = "bEnableModeSwitchKeys"))
	FKey SwitchFirstPersonKey = EKeys::One;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input", meta = (EditCondition = "bEnableModeSwitchKeys"))
	FKey SwitchThirdPersonKey = EKeys::Two;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input", meta = (EditCondition = "bEnableModeSwitchKeys"))
	FKey SwitchVRKey = EKeys::Three;

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
	TObjectPtr<UInputAction> VRTeleportAimLeftAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switchable Pawn|Input")
	TObjectPtr<UInputAction> VRTeleportAimRightAction = nullptr;

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
	void HandleVRTeleportAimLeftStarted(const FInputActionValue& Value);
	void HandleVRTeleportAimLeftCompleted(const FInputActionValue& Value);
	void HandleVRTeleportAimRightStarted(const FInputActionValue& Value);
	void HandleVRTeleportAimRightCompleted(const FInputActionValue& Value);
	void HandleVRTeleportConfirm(const FInputActionValue& Value);
	bool CanEnterVRMode() const;
	void ApplyModeTransition(ESwitchablePawnMode NewMode, ESwitchablePawnMode PreviousMode);
	void SetWindowedMode();
	void SetVRModeEnabled(bool bEnabled);
	void ApplyMovementSettingsToPawn(ASwitchableBaseCharacter* SwitchablePawn) const;
	void ApplyMovementSettingsToAllPawns() const;
	void ApplyVRSettingsToPawn(ASwitchableVRCharacter* SwitchableVRPawn) const;

	ASwitchableBaseCharacter* GetOrCreatePawnForMode(ESwitchablePawnMode Mode, const FSwitchablePawnRuntimeState& RuntimeState);
	TSubclassOf<ASwitchableBaseCharacter> GetPawnClassForMode(ESwitchablePawnMode Mode) const;
	ASwitchablePawnTeleportPoint* FindDefaultSwitchableTeleportPoint() const;
	ASwitchablePawnTeleportPoint* FindTeleportPointByName(FName PointName) const;
	ASwitchablePawnTeleportPoint* FindTeleportPointByIndex(int32 Index) const;
	FSwitchablePawnRuntimeState BuildInitialRuntimeState() const;

	UPROPERTY(Transient)
	TObjectPtr<ASwitchableBaseCharacter> FirstPersonPawn = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ASwitchableBaseCharacter> ThirdPersonPawn = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ASwitchableBaseCharacter> VRPawn = nullptr;
};
