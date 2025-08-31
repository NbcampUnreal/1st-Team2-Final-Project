#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "EPerceptionType.generated.h"

UENUM(BlueprintType)
enum class EPerceptionType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Blood = 1 UMETA(DisplayName = "Blood"),
	Player = 2 UMETA(DisplayName = "Player"),
	Damage = 3 UMETA(DisplayName = "Damage"),
	Finish = 4 UMETA(DisplayName = "Finish"),
};
