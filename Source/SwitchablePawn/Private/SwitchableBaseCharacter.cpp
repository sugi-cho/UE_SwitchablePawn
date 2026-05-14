#include "SwitchableBaseCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

ASwitchableBaseCharacter::ASwitchableBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;
}

void ASwitchableBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = WalkSpeed;
		Movement->JumpZVelocity = JumpVelocity;
	}
}

void ASwitchableBaseCharacter::Move(const FVector2D& MoveValue)
{
	if (!Controller || MoveValue.IsNearlyZero())
	{
		return;
	}

	const FRotator ControlYaw(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MoveValue.Y);
	AddMovementInput(Right, MoveValue.X);
}

void ASwitchableBaseCharacter::Look(const FVector2D& LookValue)
{
	if (LookValue.IsNearlyZero())
	{
		return;
	}

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(LookValue.Y);
}

void ASwitchableBaseCharacter::StartJump()
{
	Jump();
}

void ASwitchableBaseCharacter::StopJump()
{
	StopJumping();
}

void ASwitchableBaseCharacter::TeleportToSwitchableTransform(const FTransform& TargetTransform)
{
	TeleportTo(TargetTransform.GetLocation(), TargetTransform.Rotator(), false, true);
}

FSwitchablePawnRuntimeState ASwitchableBaseCharacter::CaptureRuntimeState() const
{
	FSwitchablePawnRuntimeState RuntimeState;
	RuntimeState.Transform = GetActorTransform();
	RuntimeState.Velocity = GetVelocity();
	RuntimeState.ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	return RuntimeState;
}

void ASwitchableBaseCharacter::ApplyRuntimeState(const FSwitchablePawnRuntimeState& RuntimeState)
{
	TeleportToSwitchableTransform(RuntimeState.Transform);

	if (Controller)
	{
		Controller->SetControlRotation(RuntimeState.ControlRotation);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->Velocity = RuntimeState.Velocity;
	}
}

void ASwitchableBaseCharacter::SetSwitchableActive(bool bNewActive)
{
	SetActorHiddenInGame(!bNewActive);
	SetActorEnableCollision(bNewActive);
	SetActorTickEnabled(bNewActive);
}
