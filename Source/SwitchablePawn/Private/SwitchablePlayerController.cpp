#include "SwitchablePlayerController.h"

#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "SwitchableBaseCharacter.h"
#include "SwitchableFirstPersonCharacter.h"
#include "SwitchablePawnStart.h"
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
		EnhancedInput->BindAction(VRTeleportAimAction, ETriggerEvent::Started, this, &ASwitchablePlayerController::HandleVRTeleportAimStarted);
		EnhancedInput->BindAction(VRTeleportAimAction, ETriggerEvent::Completed, this, &ASwitchablePlayerController::HandleVRTeleportAimCompleted);
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
	SwitchMode(ESwitchablePawnMode::VR);
}

void ASwitchablePlayerController::SwitchMode(ESwitchablePawnMode NewMode)
{
	APawn* PreviousPawn = GetPawn();
	FSwitchablePawnRuntimeState RuntimeState = BuildInitialRuntimeState();

	if (const ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(PreviousPawn))
	{
		RuntimeState = SwitchablePawn->CaptureRuntimeState();
	}
	else if (PreviousPawn)
	{
		RuntimeState.Transform = PreviousPawn->GetActorTransform();
		RuntimeState.Velocity = PreviousPawn->GetVelocity();
		RuntimeState.ControlRotation = GetControlRotation();
	}

	ASwitchableBaseCharacter* NewPawn = GetOrCreatePawnForMode(NewMode, RuntimeState);
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchablePawn: Failed to create pawn for mode."));
		return;
	}

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
	Possess(NewPawn);
	NewPawn->ApplyRuntimeState(RuntimeState);
	CurrentMode = NewMode;

	if (bDestroyInactivePawns && PreviousPawn && PreviousPawn != NewPawn)
	{
		PreviousPawn->Destroy();
	}
}

bool ASwitchablePlayerController::TeleportToStartPoint(FName PointName)
{
	FSwitchableTeleportPoint Point;
	if (!FindTeleportPoint(PointName, Point))
	{
		return false;
	}

	FTransform TargetTransform = Point.Transform;
	if (Point.bKeepCurrentYaw)
	{
		FRotator Rotation = TargetTransform.Rotator();
		Rotation.Yaw = GetControlRotation().Yaw;
		TargetTransform.SetRotation(Rotation.Quaternion());
	}

	if (ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(GetPawn()))
	{
		SwitchablePawn->TeleportToSwitchableTransform(TargetTransform);
		SetControlRotation(TargetTransform.Rotator());
		return true;
	}

	return false;
}

bool ASwitchablePlayerController::TeleportToStartPointByIndex(int32 Index)
{
	FSwitchableTeleportPoint Point;
	if (!FindTeleportPointByIndex(Index, Point))
	{
		return false;
	}

	FTransform TargetTransform = Point.Transform;
	if (Point.bKeepCurrentYaw)
	{
		FRotator Rotation = TargetTransform.Rotator();
		Rotation.Yaw = GetControlRotation().Yaw;
		TargetTransform.SetRotation(Rotation.Quaternion());
	}

	if (ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(GetPawn()))
	{
		SwitchablePawn->TeleportToSwitchableTransform(TargetTransform);
		SetControlRotation(TargetTransform.Rotator());
		return true;
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
	if (!VRTeleportAimAction)
	{
		VRTeleportAimAction = CreateInputAction(this, TEXT("IA_VRTeleportAim_Runtime"), EInputActionValueType::Boolean);
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
	InputMappingContext->MapKey(JumpAction, EKeys::Gamepad_FaceButton_Bottom);
	InputMappingContext->MapKey(SwitchFirstPersonAction, EKeys::F1);
	InputMappingContext->MapKey(SwitchThirdPersonAction, EKeys::F2);
	InputMappingContext->MapKey(SwitchVRAction, EKeys::F3);
	InputMappingContext->MapKey(VRTeleportAimAction, EKeys::RightMouseButton);
	InputMappingContext->MapKey(VRTeleportConfirmAction, EKeys::Gamepad_RightTrigger);
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
	if (ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(GetPawn()))
	{
		SwitchablePawn->Move(Value.Get<FVector2D>());
	}
}

void ASwitchablePlayerController::HandleLook(const FInputActionValue& Value)
{
	if (ASwitchableBaseCharacter* SwitchablePawn = Cast<ASwitchableBaseCharacter>(GetPawn()))
	{
		SwitchablePawn->Look(Value.Get<FVector2D>());
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

void ASwitchablePlayerController::HandleVRTeleportAimStarted(const FInputActionValue& Value)
{
	if (ASwitchableVRCharacter* SwitchableVRPawn = Cast<ASwitchableVRCharacter>(GetPawn()))
	{
		SwitchableVRPawn->BeginTeleportAim();
	}
}

void ASwitchablePlayerController::HandleVRTeleportAimCompleted(const FInputActionValue& Value)
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
		(*StoredPawn)->SetSwitchableActive(false);
	}
	return *StoredPawn;
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

ASwitchablePawnStart* ASwitchablePlayerController::FindDefaultSwitchableStart() const
{
	ASwitchablePawnStart* FirstStart = nullptr;

	for (TActorIterator<ASwitchablePawnStart> It(GetWorld()); It; ++It)
	{
		ASwitchablePawnStart* Start = *It;
		if (!FirstStart)
		{
			FirstStart = Start;
		}
		if (Start->bUseAsDefaultStart)
		{
			return Start;
		}
	}

	return FirstStart;
}

bool ASwitchablePlayerController::FindTeleportPoint(FName PointName, FSwitchableTeleportPoint& OutPoint) const
{
	for (TActorIterator<ASwitchablePawnStart> It(GetWorld()); It; ++It)
	{
		if (It->FindPresetPointByName(PointName, OutPoint))
		{
			return true;
		}
	}

	return false;
}

bool ASwitchablePlayerController::FindTeleportPointByIndex(int32 Index, FSwitchableTeleportPoint& OutPoint) const
{
	if (ASwitchablePawnStart* Start = FindDefaultSwitchableStart())
	{
		return Start->FindPresetPointByIndex(Index, OutPoint);
	}

	return false;
}

FSwitchablePawnRuntimeState ASwitchablePlayerController::BuildInitialRuntimeState() const
{
	FSwitchablePawnRuntimeState RuntimeState;
	RuntimeState.ControlRotation = GetControlRotation();

	if (ASwitchablePawnStart* Start = FindDefaultSwitchableStart())
	{
		RuntimeState.Transform = Start->GetStartTransform();
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
