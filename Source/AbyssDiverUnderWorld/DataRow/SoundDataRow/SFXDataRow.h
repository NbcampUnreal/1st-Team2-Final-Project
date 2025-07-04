#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "SFXDataRow.generated.h"

UENUM(BlueprintType)
enum class ESFX : uint8
{
	Sound0,
	RefillOxygen,
	Breath,
	DropItem,
	Explosion,
	Hit,
	NVOn,
	NVOff,
	Equip,
	UnEquip,
	CompleteMine,
	Pickup,
	OpenTablet,
	SubmitOre,
	SendDrone,
	FireHarpoon,
	DPVOn,
	DPVOff,
	ActivateDrone,
	Submarine,
	LaserSound,
	WalkFootstep,
	LandFootstep,
	ShieldBroken,
	ShieldHit,
	DamageTaken,
	UnderwaterMovement,
	UnderwaterSprint,
	IdleBreath,
	MoveBreath,
	Resurrection,
	UseShield,
	GroggyStart,
	GroggyMiddle,
	GroggyEnd,
	ShopDoorOpen,
	ShopDoorClose,
	WhaleSound,
	Max UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FSFXDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	ESFX SoundType;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> Sound;
};
