#pragma once

#include "CoreMinimal.h"

namespace BlackboardKeys
{
	inline const FName MonsterStateKey = TEXT("MonsterState");
	inline const FName PerceptionTypeKey = TEXT("EPerceptionType");

	inline const FName TargetPlayerKey = TEXT("TargetPlayer");
	inline const FName InvestigateLocationKey = TEXT("InvestigateLocation");
	inline const FName TargetLocationKey = TEXT("TargetLocation");

	inline const FName bIsChasingKey = TEXT("bIsChasing");
	inline const FName bCanAttackKey = TEXT("bCanAttack");
	
	namespace GobleFish
	{
		inline const FName bInMeleeRangeKey = TEXT("bInMeleeRange");
		inline const FName bInRangedRangeKey = TEXT("bInRangedRange");
	}

	namespace HorrorCreature
	{
		inline const FName bIsPlayerSwallowKey = TEXT("bIsPlayerSwallow");
		inline const FName FleeLocationKey = TEXT("FleeLocation");
	}

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