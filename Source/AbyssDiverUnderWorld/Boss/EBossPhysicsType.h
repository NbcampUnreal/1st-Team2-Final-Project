#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "EBossPhysicsType.generated.h"

UENUM(BlueprintType)
enum class EBossPhysicsType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	SimulatePhysics = 1 UMETA(DisplayName = "SimulatePhysics")
};