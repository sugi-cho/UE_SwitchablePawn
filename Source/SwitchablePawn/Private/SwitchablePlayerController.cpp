#include "SwitchablePlayerController.h"

#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerStart.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "SwitchableBaseCharacter.h"
#include "SwitchableFirstPersonCharacter.h"
#include "SwitchablePawnTeleportPoint.h"
#include "SwitchableThirdPersonCharacter.h"
#include "SwitchableVRCharacter.h"

namespace
{
UInputAction* CreateInputAction(UObject* Outer, FName Name, EInputActionValueType ValueType)
{
	UInputAction* Action = NewObject<UInputAction>(Outer, Name);
	Action->ValueType = ValueType;
	return Action;
}

void AddNegateModifier(UInputMappingContext* Context, FEnhancedActionKeyMapping& Mapping, bool bX, bool bY)
{
	UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(Context);
	Negate->bX = bX;
	Negate->bY = bY;
	Negate->bZ = false;
	Mapping.Modifiers.Add(Negate);
}

void AddSwizzleModifier(UInputMappingContext* Context, FEnhancedActionKeyMapping& Mapping)
{
	UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(Context);
	Swizzle->Order = EInputAxisSwizzle::YXZ;
	Mapping.Modifiers.Add(Swizzle);
}
}

ASwitchablePlayerController::ASwitchablePlayerController()
{
	FirstPersonClass = ASwitchableFirstPersonCharacter::StaticClass();
	ThirdPersonClass = ASwitchableThirdPersonCharacter::StaticClass();
	VRClass = ASwitchableVRCharacter::StaticClass();
}

void ASwitchablePlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnsureFallbackInputAssets();
	AddMappingContextToLocalPlayer();
	SwitchMode(DefaultMode);
}

void ASwitchablePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	EnsureFallbackInputAssets();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASwitchablePlayerController::HandleMove);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASwitchablePlayerController::HandleLook);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ASwitchablePlayerController::HandleJumpStarted);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASwitchablePlayerController::HandleJumpCompleted);
		EnhancedInput->BindAction(SwitchFirstPersonAction, ETriggerEvent::Started, this, &ASwitchablePlayerController::SwitchToFirstPerson);
		EnhancedInput->BindAction(SwitchThirdPersonAction, ETriggerEvent::Started, this, &ASwitchablePlayerController::SwitchToThirdPerson);
		EnhancedInput->BindAction(SwitchVRAction, ETriggerEvent::Started, this, &ASwitchablePlayerController::SwitchToVR);
		EnhancedInput->BindAction(VRTeleportAimLeftAction, ETriggerEvent::Started, this, &ASwitchablePlayerController::HandleVRTeleportAimLeftStarted);
		EnhancedInput->BindAction(VRTeleportAimLeftAction, ETriggerEvent::Completed, this, &ASwitchablePlayerController::HandleVRTeleportAimLeftCompleted);
		EnhancedInput->BindAction(VRTeleportAimRightAction, ETriggerEvent::Started, this, &ASwitchablePlayerController::HandleVRTeleportAimRightStarted);
		EnhancedInput->BindAction(VRTeleportAimRightAction, ETriggerEvent::Completed, this, &ASwitchablePlayerController::HandleVRTeleportAimRightCompleted);
		EnhancedInput->BindAction(VRTeleportConfirmAction, ETriggerEvent::Started, this, &ASwitchablePlayerController::HandleVRTeleportConfirm);
	}
}

void ASwitchablePlayerController::SwitchToFirstPerson()
{
	SwitchMode(ESwitchablePawnMode::FirstPerson);
}

void ASwitchablePlayerController::SwitchToThirdPerson()
{
	SwitchMode(ESwitchablePawnMode::ThirdPerson);
}

void ASwitchablePlayerController::SwitchToVR()
{
	if (!CanEnterVRMode())
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchablePawn: VR device is not available."));
		return;
	}

	SwitchMode(ESwitchablePawnMode::VR);
}

void ASwitchablePlayerController::SwitchMode(ESwitchablePawnMode NewMode)
{
	APawn* PreviousPawn = GetPawn();
	const ESwitchablePawnMode PreviousMode = CurrentMode;
	FSwitchablePawnRuntimeState RuntimeState = BuildInitialRuntimeState();

	const bool bPreviousPawnIsManaged =
		PreviousPawn == FirstPersonPawn ||
		PreviousPawn == ThirdPersonPawn ||
		PreviousPawn == VRPawn;

	if (bPreviousPawnIsManaged)
	{
		if (const ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(PreviousPawn))
		{
			RuntimeState = SwitchablePawn->CaptureRuntimeState();
		}
	}
	else if (PreviousPawn)
	{
		// First launch pawn belongs to GameMode; do not inherit its transform.
	}

	ASwitchableBaseCharacter* NewPawn = GetOrCreatePawnForMode(NewMode, RuntimeState);
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchablePawn: Failed to create pawn for mode."));
		return;
	}

	OnModeWillChange(PreviousMode, NewMode, PreviousPawn, NewPawn);
	ApplyModeTransition(NewMode, PreviousMode);

	if (PreviousPawn && PreviousPawn != NewPawn)
	{
		if (ASwitchableBaseCharacter* PreviousSwitchablePawn = Cast<ASwitchableBaseCharacter>(PreviousPawn))
		{
			PreviousSwitchablePawn->SetSwitchableActive(false);
		}
		else
		{
			PreviousPawn->SetActorHiddenInGame(true);
			PreviousPawn->SetActorEnableCollision(false);
		}
	}

	NewPawn->SetSwitchableActive(true);
	ApplyMovementSettingsToPawn(NewPawn);
	ApplyVRSettingsToPawn(Cast<ASwitchableVRCharacter>(NewPawn));
	Possess(NewPawn);
	NewPawn->ApplyRuntimeState(RuntimeState);
	CurrentMode = NewMode;
	OnModeChanged(PreviousMode, NewMode, PreviousPawn, NewPawn);

	if (bDestroyInactivePawns && PreviousPawn && PreviousPawn != NewPawn)
	{
		PreviousPawn->Destroy();
	}
}

void ASwitchablePlayerController::SetConstrainMovementToNavMesh(bool bNewConstrain)
{
	bConstrainMovementToNavMesh = bNewConstrain;
	ApplyMovementSettingsToAllPawns();
}

void ASwitchablePlayerController::SetAffectedByGravity(bool bNewAffectedByGravity)
{
	bAffectedByGravity = bNewAffectedByGravity;
	ApplyMovementSettingsToAllPawns();
}

void ASwitchablePlayerController::ApplyMovementSettingsToAllPawns() const
{
	ApplyMovementSettingsToPawn(Cast<ASwitchableBaseCharacter>(GetPawn()));
	if (IsValid(FirstPersonPawn))
	{
		ApplyMovementSettingsToPawn(FirstPersonPawn);
	}
	if (IsValid(ThirdPersonPawn))
	{
		ApplyMovementSettingsToPawn(ThirdPersonPawn);
	}
	if (IsValid(VRPawn))
	{
		ApplyMovementSettingsToPawn(VRPawn);
	}
}

void ASwitchablePlayerController::SetVRTeleportMovementEnabled(bool bNewEnabled)
{
	bEnableVRTeleportMovement = bNewEnabled;

	ApplyVRSettingsToPawn(Cast<ASwitchableVRCharacter>(GetPawn()));
	if (IsValid(VRPawn))
	{
		ApplyVRSettingsToPawn(Cast<ASwitchableVRCharacter>(VRPawn));
	}
}

void ASwitchablePlayerController::SetMovementInputEnabled(bool bNewEnabled)
{
	bEnableMovementInput = bNewEnabled;
}

void ASwitchablePlayerController::SetLookInputEnabled(bool bNewEnabled)
{
	bEnableLookInput = bNewEnabled;
}

void ASwitchablePlayerController::ApplyModeTransition(ESwitchablePawnMode NewMode, ESwitchablePawnMode PreviousMode)
{
	const bool bEnteringVR = NewMode == ESwitchablePawnMode::VR && PreviousMode != ESwitchablePawnMode::VR;
	const bool bLeavingVR = PreviousMode == ESwitchablePawnMode::VR && NewMode != ESwitchablePawnMode::VR;

	if (bEnteringVR)
	{
		SetVRModeEnabled(true);
	}

	if (bLeavingVR)
	{
		SetVRModeEnabled(false);
		SetWindowedMode();
	}
}

bool ASwitchablePlayerController::CanEnterVRMode() const
{
	return UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayConnected();
}

void ASwitchablePlayerController::SetWindowedMode()
{
	if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
	{
		GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
		GameUserSettings->ApplySettings(false);
	}
}

void ASwitchablePlayerController::SetVRModeEnabled(bool bEnabled)
{
	UHeadMountedDisplayFunctionLibrary::EnableHMD(bEnabled);

	if (GEngine && GEngine->StereoRenderingDevice.IsValid())
	{
		GEngine->StereoRenderingDevice->EnableStereo(bEnabled);
	}

	if (bEnabled)
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::LocalFloor);
	}
}

bool ASwitchablePlayerController::TeleportToStartPoint()
{
	return TeleportToPoint(FindDefaultSwitchableTeleportPoint());
}

bool ASwitchablePlayerController::TeleportToPointByName(FName PointName)
{
	if (ASwitchablePawnTeleportPoint* TeleportPoint = FindTeleportPointByName(PointName))
	{
		return TeleportToPoint(TeleportPoint);
	}

	return false;
}

bool ASwitchablePlayerController::TeleportToPoint(ASwitchablePawnTeleportPoint* TeleportPoint)
{
	if (!TeleportPoint)
	{
		return false;
	}

	const FTransform TargetTransform = TeleportPoint->GetTeleportTransform();
	if (ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(GetPawn()))
	{
		SwitchablePawn->TeleportToSwitchableTransform(TargetTransform);
		SetControlRotation(TargetTransform.Rotator());
		return true;
	}

	return false;
}

bool ASwitchablePlayerController::TeleportToPointByIndex(int32 Index)
{
	if (ASwitchablePawnTeleportPoint* TeleportPoint = FindTeleportPointByIndex(Index))
	{
		return TeleportToPoint(TeleportPoint);
	}

	return false;
}

void ASwitchablePlayerController::EnsureFallbackInputAssets()
{
	if (!MoveAction)
	{
		MoveAction = CreateInputAction(this, TEXT("IA_Move_Runtime"), EInputActionValueType::Axis2D);
	}
	if (!LookAction)
	{
		LookAction = CreateInputAction(this, TEXT("IA_Look_Runtime"), EInputActionValueType::Axis2D);
	}
	if (!JumpAction)
	{
		JumpAction = CreateInputAction(this, TEXT("IA_Jump_Runtime"), EInputActionValueType::Boolean);
	}
	if (!SwitchFirstPersonAction)
	{
		SwitchFirstPersonAction = CreateInputAction(this, TEXT("IA_SwitchFirstPerson_Runtime"), EInputActionValueType::Boolean);
	}
	if (!SwitchThirdPersonAction)
	{
		SwitchThirdPersonAction = CreateInputAction(this, TEXT("IA_SwitchThirdPerson_Runtime"), EInputActionValueType::Boolean);
	}
	if (!SwitchVRAction)
	{
		SwitchVRAction = CreateInputAction(this, TEXT("IA_SwitchVR_Runtime"), EInputActionValueType::Boolean);
	}
	if (!VRTeleportAimLeftAction)
	{
		VRTeleportAimLeftAction = CreateInputAction(this, TEXT("IA_VRTeleportAimLeft_Runtime"), EInputActionValueType::Boolean);
	}
	if (!VRTeleportAimRightAction)
	{
		VRTeleportAimRightAction = CreateInputAction(this, TEXT("IA_VRTeleportAimRight_Runtime"), EInputActionValueType::Boolean);
	}
	if (!VRTeleportConfirmAction)
	{
		VRTeleportConfirmAction = CreateInputAction(this, TEXT("IA_VRTeleportConfirm_Runtime"), EInputActionValueType::Boolean);
	}
	if (!InputMappingContext)
	{
		InputMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_SwitchablePawn_Runtime"));
		AddDefaultMappings();
	}
}

void ASwitchablePlayerController::AddDefaultMappings()
{
	if (!InputMappingContext)
	{
		return;
	}

	FEnhancedActionKeyMapping& MoveForward = InputMappingContext->MapKey(MoveAction, EKeys::W);
	AddSwizzleModifier(InputMappingContext, MoveForward);

	FEnhancedActionKeyMapping& MoveBackward = InputMappingContext->MapKey(MoveAction, EKeys::S);
	AddSwizzleModifier(InputMappingContext, MoveBackward);
	AddNegateModifier(InputMappingContext, MoveBackward, false, true);

	FEnhancedActionKeyMapping& MoveRight = InputMappingContext->MapKey(MoveAction, EKeys::D);
	AddNegateModifier(InputMappingContext, MoveRight, false, false);

	FEnhancedActionKeyMapping& MoveLeft = InputMappingContext->MapKey(MoveAction, EKeys::A);
	AddNegateModifier(InputMappingContext, MoveLeft, true, false);

	InputMappingContext->MapKey(LookAction, EKeys::Mouse2D);
	InputMappingContext->MapKey(JumpAction, EKeys::SpaceBar);
	InputMappingContext->MapKey(MoveAction, EKeys::OculusTouch_Left_Thumbstick_2D);
	InputMappingContext->MapKey(MoveAction, EKeys::OculusTouch_Right_Thumbstick_2D);
	InputMappingContext->MapKey(MoveAction, EKeys::ValveIndex_Left_Thumbstick_2D);
	InputMappingContext->MapKey(MoveAction, EKeys::ValveIndex_Right_Thumbstick_2D);
	InputMappingContext->MapKey(MoveAction, EKeys::MixedReality_Left_Thumbstick_2D);
	InputMappingContext->MapKey(MoveAction, EKeys::MixedReality_Right_Thumbstick_2D);
	if (bEnableModeSwitchKeys)
	{
		if (SwitchFirstPersonKey.IsValid())
		{
			InputMappingContext->MapKey(SwitchFirstPersonAction, SwitchFirstPersonKey);
		}
		if (SwitchThirdPersonKey.IsValid())
		{
			InputMappingContext->MapKey(SwitchThirdPersonAction, SwitchThirdPersonKey);
		}
		if (SwitchVRKey.IsValid())
		{
			InputMappingContext->MapKey(SwitchVRAction, SwitchVRKey);
		}
	}
	InputMappingContext->MapKey(VRTeleportAimLeftAction, EKeys::OculusTouch_Left_Trigger_Click);
	InputMappingContext->MapKey(VRTeleportAimLeftAction, EKeys::ValveIndex_Left_Trigger_Click);
	InputMappingContext->MapKey(VRTeleportAimLeftAction, EKeys::MixedReality_Left_Trigger_Click);
	InputMappingContext->MapKey(VRTeleportAimRightAction, EKeys::OculusTouch_Right_Trigger_Click);
	InputMappingContext->MapKey(VRTeleportAimRightAction, EKeys::ValveIndex_Right_Trigger_Click);
	InputMappingContext->MapKey(VRTeleportAimRightAction, EKeys::MixedReality_Right_Trigger_Click);
}

void ASwitchablePlayerController::AddMappingContextToLocalPlayer()
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->AddMappingContext(InputMappingContext, InputMappingPriority);
		}
	}
}

void ASwitchablePlayerController::HandleMove(const FInputActionValue& Value)
{
	if (!bEnableMovementInput)
	{
		return;
	}

	if (ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(GetPawn()))
	{
		SwitchablePawn->Move(Value.Get<FVector2D>());
	}
}

void ASwitchablePlayerController::HandleLook(const FInputActionValue& Value)
{
	if (!bEnableLookInput)
	{
		return;
	}

	if (ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(GetPawn()))
	{
		FVector2D LookValue = Value.Get<FVector2D>();
		LookValue.Y *= -1.0f;
		SwitchablePawn->Look(LookValue);
	}
}

void ASwitchablePlayerController::HandleJumpStarted(const FInputActionValue& Value)
{
	if (ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(GetPawn()))
	{
		SwitchablePawn->StartJump();
	}
}

void ASwitchablePlayerController::HandleJumpCompleted(const FInputActionValue& Value)
{
	if (ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(GetPawn()))
	{
		SwitchablePawn->StopJump();
	}
}

void ASwitchablePlayerController::HandleVRTeleportAimLeftStarted(const FInputActionValue& Value)
{
	if (ASwitchableVRCharacter* SwitchableVRPawn = Cast<ASwitchableVRCharacter>(GetPawn()))
	{
		SwitchableVRPawn->BeginTeleportAim(true);
	}
}

void ASwitchablePlayerController::HandleVRTeleportAimLeftCompleted(const FInputActionValue& Value)
{
	if (ASwitchableVRCharacter* SwitchableVRPawn = Cast<ASwitchableVRCharacter>(GetPawn()))
	{
		SwitchableVRPawn->ConfirmTeleport();
	}
}

void ASwitchablePlayerController::HandleVRTeleportAimRightStarted(const FInputActionValue& Value)
{
	if (ASwitchableVRCharacter* SwitchableVRPawn = Cast<ASwitchableVRCharacter>(GetPawn()))
	{
		SwitchableVRPawn->BeginTeleportAim(false);
	}
}

void ASwitchablePlayerController::HandleVRTeleportAimRightCompleted(const FInputActionValue& Value)
{
	if (ASwitchableVRCharacter* SwitchableVRPawn = Cast<ASwitchableVRCharacter>(GetPawn()))
	{
		SwitchableVRPawn->ConfirmTeleport();
	}
}

void ASwitchablePlayerController::HandleVRTeleportConfirm(const FInputActionValue& Value)
{
	if (ASwitchableVRCharacter* SwitchableVRPawn = Cast<ASwitchableVRCharacter>(GetPawn()))
	{
		SwitchableVRPawn->ConfirmTeleport();
	}
}

ASwitchableBaseCharacter* ASwitchablePlayerController::GetOrCreatePawnForMode(ESwitchablePawnMode Mode, const FSwitchablePawnRuntimeState& RuntimeState)
{
	TObjectPtr<ASwitchableBaseCharacter>* StoredPawn = nullptr;
	switch (Mode)
	{
	case ESwitchablePawnMode::FirstPerson:
		StoredPawn = &FirstPersonPawn;
		break;
	case ESwitchablePawnMode::ThirdPerson:
		StoredPawn = &ThirdPersonPawn;
		break;
	case ESwitchablePawnMode::VR:
		StoredPawn = &VRPawn;
		break;
	default:
		break;
	}

	if (!StoredPawn)
	{
		return nullptr;
	}

	if (IsValid(*StoredPawn))
	{
		return *StoredPawn;
	}

	TSubclassOf<ASwitchableBaseCharacter> PawnClass = GetPawnClassForMode(Mode);
	if (!PawnClass || !GetWorld())
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetPawn();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	*StoredPawn = GetWorld()->SpawnActor<ASwitchableBaseCharacter>(PawnClass, RuntimeState.Transform, SpawnParams);
	if (*StoredPawn)
	{
		ApplyMovementSettingsToPawn(*StoredPawn);
		ApplyVRSettingsToPawn(Cast<ASwitchableVRCharacter>(*StoredPawn));
		(*StoredPawn)->SetSwitchableActive(false);
	}
	return *StoredPawn;
}

void ASwitchablePlayerController::ApplyMovementSettingsToPawn(ASwitchableBaseCharacter* SwitchablePawn) const
{
	if (SwitchablePawn)
	{
		SwitchablePawn->SetConstrainMovementToNavMesh(bConstrainMovementToNavMesh);
		SwitchablePawn->SetAffectedByGravity(bAffectedByGravity);

		if (ASwitchableVRCharacter* SwitchableVRPawn = Cast<ASwitchableVRCharacter>(SwitchablePawn))
		{
			SwitchableVRPawn->SetProjectTeleportToNavigation(bConstrainMovementToNavMesh);
		}
	}
}

void ASwitchablePlayerController::ApplyVRSettingsToPawn(ASwitchableVRCharacter* SwitchableVRPawn) const
{
	if (SwitchableVRPawn)
	{
		SwitchableVRPawn->SetTeleportMovementEnabled(bEnableVRTeleportMovement);
	}
}

TSubclassOf<ASwitchableBaseCharacter> ASwitchablePlayerController::GetPawnClassForMode(ESwitchablePawnMode Mode) const
{
	switch (Mode)
	{
	case ESwitchablePawnMode::FirstPerson:
		return FirstPersonClass;
	case ESwitchablePawnMode::ThirdPerson:
		return ThirdPersonClass;
	case ESwitchablePawnMode::VR:
		return VRClass;
	default:
		return nullptr;
	}
}

ASwitchablePawnTeleportPoint* ASwitchablePlayerController::FindDefaultSwitchableTeleportPoint() const
{
	ASwitchablePawnTeleportPoint* FirstTeleportPoint = nullptr;

	for (TActorIterator<ASwitchablePawnTeleportPoint> It(GetWorld()); It; ++It)
	{
		ASwitchablePawnTeleportPoint* TeleportPoint = *It;
		if (!FirstTeleportPoint)
		{
			FirstTeleportPoint = TeleportPoint;
		}
		if (TeleportPoint->bSetAsDefaultStart)
		{
			return TeleportPoint;
		}
	}

	return FirstTeleportPoint;
}

ASwitchablePawnTeleportPoint* ASwitchablePlayerController::FindTeleportPointByName(FName PointName) const
{
	for (TActorIterator<ASwitchablePawnTeleportPoint> It(GetWorld()); It; ++It)
	{
		if (It->GetFName() == PointName)
		{
			return *It;
		}
	}

	return nullptr;
}

ASwitchablePawnTeleportPoint* ASwitchablePlayerController::FindTeleportPointByIndex(int32 Index) const
{
	int32 CurrentIndex = 0;
	for (TActorIterator<ASwitchablePawnTeleportPoint> It(GetWorld()); It; ++It, ++CurrentIndex)
	{
		if (CurrentIndex == Index)
		{
			return *It;
		}
	}

	return nullptr;
}

FSwitchablePawnRuntimeState ASwitchablePlayerController::BuildInitialRuntimeState() const
{
	FSwitchablePawnRuntimeState RuntimeState;
	RuntimeState.ControlRotation = GetControlRotation();

	if (ASwitchablePawnTeleportPoint* TeleportPoint = FindDefaultSwitchableTeleportPoint())
	{
		RuntimeState.Transform = TeleportPoint->GetTeleportTransform();
		RuntimeState.ControlRotation = RuntimeState.Transform.Rotator();
		return RuntimeState;
	}

	if (const APawn* ExistingPawn = GetPawn())
	{
		RuntimeState.Transform = ExistingPawn->GetActorTransform();
		return RuntimeState;
	}

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		RuntimeState.Transform = It->GetActorTransform();
		RuntimeState.ControlRotation = RuntimeState.Transform.Rotator();
		return RuntimeState;
	}

	return RuntimeState;
}
