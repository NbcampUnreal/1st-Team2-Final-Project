#pragma once

#include "CoreMinimal.h"

namespace BlackboardKeys
{
	inline const FName MonsterStateKey = TEXT("MonsterState");
	inline const FName PerceptionTypeKey = TEXT("EPerceptionType");

	inline const FName TargetPlayerKey = TEXT("TargetPlayer");

	inline const FName InvestigateLocationKey = TEXT("InvestigateLocation");
	inline const FName BloodOccurredLocationKey = TEXT("BloodOccurredLocation");
	inline const FName PatrolLocationKey = TEXT("PatrolLocation");
	inline const FName TargetLocationKey = TEXT("TargetLocation");

	inline const FName bIsDetectBloodKey = TEXT("bIsDetectBlood");
	inline const FName bIsChasingKey = TEXT("bIsChasing");
	inline const FName bIsChasingPlayerKey = TEXT("bIsChasingPlayer");
	inline const FName bIsChasingBloodKey = TEXT("bIsChasingBlood");
	inline const FName bIsPlayerHiddenKey = TEXT("bIsPlayerHidden");
	inline const FName bCanAttackKey = TEXT("bCanAttack");
	inline const FName bHasAttackedKey = TEXT("bHasAttacked");
	inline const FName bHasSeenPlayerKey = TEXT("bHasSeenPlayer");
	inline const FName bHasDetectedPlayerKey = TEXT("bHasDetectedPlayer");

	namespace AlienShark
	{
		inline const FName bCanChaseKey = TEXT("bCanChase");
	}

	namespace EyeStalker
	{
		inline const FName bIsAttackingKey = TEXT("bIsAttacking");
		inline const FName bHasDetectedKey = TEXT("bHasDetected");
	}
}