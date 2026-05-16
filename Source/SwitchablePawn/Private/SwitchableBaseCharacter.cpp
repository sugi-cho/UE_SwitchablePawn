#include "SwitchableBaseCharacter.h"

#include "SwitchableCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/NavLinkProxy.h"
#include "NavLinkCustomComponent.h"
#include "EngineUtils.h"

ASwitchableBaseCharacter::ASwitchableBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USwitchableCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
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

	RefreshMovementModeForNavigation();
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
	const FVector MoveDirection = (Forward * MoveValue.Y) + (Right * MoveValue.X);

	if (bConstrainMovementToNavMesh && TryTraverseNavLinkProxy(MoveDirection))
	{
		return;
	}

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

	RefreshMovementModeForNavigation();
}

void ASwitchableBaseCharacter::SetSwitchableActive(bool bNewActive)
{
	SetActorHiddenInGame(!bNewActive);
	SetActorEnableCollision(bNewActive);
	SetActorTickEnabled(bNewActive);
}

void ASwitchableBaseCharacter::SetConstrainMovementToNavMesh(bool bNewConstrain)
{
	bConstrainMovementToNavMesh = bNewConstrain;
	RefreshMovementModeForNavigation();
}

void ASwitchableBaseCharacter::RefreshMovementModeForNavigation()
{
	if (USwitchableCharacterMovementComponent* Movement = Cast<USwitchableCharacterMovementComponent>(GetCharacterMovement()))
	{
		Movement->SetNavMeshMovementEnabled(bConstrainMovementToNavMesh);
		Movement->SetMovementMode(bConstrainMovementToNavMesh ? MOVE_NavWalking : MOVE_Walking);
	}
}

bool ASwitchableBaseCharacter::TryTraverseNavLinkProxy(const FVector& MoveDirection)
{
	if (!bUseNavLinkProxyTraversal || !bConstrainMovementToNavMesh || MoveDirection.IsNearlyZero() || !GetWorld())
	{
		return false;
	}

	TArray<ANavLinkProxy*> Proxies;
	if (NavLinkProxies.Num() > 0)
	{
		for (const TObjectPtr<ANavLinkProxy>& Proxy : NavLinkProxies)
		{
			if (Proxy)
			{
				Proxies.Add(Proxy.Get());
			}
		}
	}
	else
	{
		for (TActorIterator<ANavLinkProxy> It(GetWorld()); It; ++It)
		{
			Proxies.Add(*It);
		}
	}

	if (Proxies.IsEmpty())
	{
		return false;
	}

	const FVector CurrentLocation = GetActorLocation();
	const float ActivationRadiusSq = FMath::Square(NavLinkActivationRadius);
	const float CapsuleHalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.0f;
	const FVector MoveDirection2D = FVector(MoveDirection.X, MoveDirection.Y, 0.0f).GetSafeNormal();

	auto TryTraverseLink = [&](const FVector& LinkStart, const FVector& LinkEnd) -> bool
	{
		const FVector Start2D(LinkStart.X, LinkStart.Y, 0.0f);
		const FVector End2D(LinkEnd.X, LinkEnd.Y, 0.0f);
		const FVector LinkDirection2D = (End2D - Start2D).GetSafeNormal();
		if (LinkDirection2D.IsNearlyZero())
		{
			return false;
		}

		const bool bNearStart = FVector::DistSquared2D(CurrentLocation, LinkStart) <= ActivationRadiusSq;
		const bool bNearEnd = FVector::DistSquared2D(CurrentLocation, LinkEnd) <= ActivationRadiusSq;

		if (bNearStart && FVector::DotProduct(MoveDirection2D, LinkDirection2D) > 0.5f)
		{
			TeleportToSwitchableTransform(FTransform(Controller ? Controller->GetControlRotation() : GetActorRotation(), LinkEnd + FVector(0.0f, 0.0f, CapsuleHalfHeight)));
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->Velocity = FVector::ZeroVector;
			}
			return true;
		}

		if (bNearEnd && FVector::DotProduct(MoveDirection2D, -LinkDirection2D) > 0.5f)
		{
			TeleportToSwitchableTransform(FTransform(Controller ? Controller->GetControlRotation() : GetActorRotation(), LinkStart + FVector(0.0f, 0.0f, CapsuleHalfHeight)));
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->Velocity = FVector::ZeroVector;
			}
			return true;
		}

		return false;
	};

	for (ANavLinkProxy* Proxy : Proxies)
	{
		if (!IsValid(Proxy))
		{
			continue;
		}

		const FTransform ProxyTransform = Proxy->GetActorTransform();

		for (const FNavigationLink& Link : Proxy->PointLinks)
		{
			const FVector LinkStart = ProxyTransform.TransformPosition(Link.Left);
			const FVector LinkEnd = ProxyTransform.TransformPosition(Link.Right);
			if (TryTraverseLink(LinkStart, LinkEnd))
			{
				return true;
			}
		}

		if (UNavLinkCustomComponent* SmartLinkComp = Proxy->GetSmartLinkComp())
		{
			FVector LinkStart = FVector::ZeroVector;
			FVector LinkEnd = FVector::ZeroVector;
			ENavLinkDirection::Type LinkDirection;
			SmartLinkComp->GetLinkData(LinkStart, LinkEnd, LinkDirection);

			LinkStart = ProxyTransform.TransformPosition(LinkStart);
			LinkEnd = ProxyTransform.TransformPosition(LinkEnd);
			if (TryTraverseLink(LinkStart, LinkEnd))
			{
				return true;
			}
		}
	}

	return false;
}
