#include "SwitchableCharacterMovementComponent.h"

void USwitchableCharacterMovementComponent::SetNavMeshMovementEnabled(bool bEnabled)
{
	bProjectNavMeshWalking = bEnabled;
	bProjectNavMeshOnBothWorldChannels = bEnabled;
	bSlideAlongNavMeshEdge = bEnabled;
}
