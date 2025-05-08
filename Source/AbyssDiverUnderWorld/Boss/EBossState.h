#pragma once

#include "CoreMinimal.h"
#include "EBossState.generated.h"

UENUM(BlueprintType)
enum class EBossState : uint8
{
	Idle = 0 UMETA(DisplayName = "Idle"),
	Move = 1 UMETA(DisplayName = "Move"),
	Attack = 2 UMETA(DisplayName = "Attack"),
	Chase = 3 UMETA(DisplayName = "Chase"),
	Investigate = 4 UMETA(DisplayName = "Investigate"),
};
