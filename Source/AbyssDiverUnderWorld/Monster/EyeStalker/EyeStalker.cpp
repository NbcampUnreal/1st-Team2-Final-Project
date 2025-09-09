#include "Monster/EyeStalker/EyeStalker.h"

#include "Monster/Components/AquaticMovementComponent.h"
#include "Monster/Components/TickControlComponent.h"

AEyeStalker::AEyeStalker()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEyeStalker::BeginPlay()
{
	Super::BeginPlay();

	if (AquaticMovementComponent)
	{
		TickControlComponent->UnregisterComponent(AquaticMovementComponent);
	}
}

void AEyeStalker::M_SetEyeOpenness_Implementation(float Openness)
{
	SetEyeOpenness(Openness);
}

void AEyeStalker::M_SetTargetPlayer_Implementation(AUnderwaterCharacter* Player)
{
	SetTargetPlayer(Player);
}

void AEyeStalker::M_SetDetectedState_Implementation(bool bDetected)
{
	SetDetectedState(bDetected);
}
