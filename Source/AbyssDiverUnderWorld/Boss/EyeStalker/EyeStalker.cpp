#include "Boss/EyeStalker/EyeStalker.h"

AEyeStalker::AEyeStalker()
{
	PrimaryActorTick.bCanEverTick = false;
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
