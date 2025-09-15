#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "SafeZoneTriggerBox.generated.h"

class AUnderwaterCharacter;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASafeZoneTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

public:
	ASafeZoneTriggerBox();

public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

#pragma region Method

protected:

	/** Character가 SafeZone 안에 있는지 체크 */
	bool IsActorInZone(AUnderwaterCharacter* Character);

	/** Overlapping Characters 중에서 SafeZone 안에 있는지 주기적으로 체크 */
	void UpdateCharactersInZone();
	
#pragma endregion

#pragma region Variable

protected:

	/** SafeZone 체크 주기 (초) */
	UPROPERTY(EditAnywhere, Category = "SafeZone", meta = (ClampMin = "0.001"))
	float UpdateInterval = 0.033f;

	/** SafeZone 체크용 Timer Handle */
	FTimerHandle UpdateTimerHandle;

	/** SafeZone의 위쪽 비율 (0.0 ~ 1.0), 1.0을 하면 Top 위치를 확인하고 0.0이 되면 Center를 기준으로 판단 */
	UPROPERTY(EditAnywhere, Category = "SafeZone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UpperZoneRatio = 0.8f;

	/** SafeZone의 아래쪽 비율 (0.0 ~ 1.0), 1.0을 하면 Bottom 위치를 확인하고 0.0이 되면 Center를 기준으로 판단 */
	UPROPERTY(EditAnywhere, Category = "SafeZone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowerZoneRatio = 0.6f;

	/** SafeZone의 반지름 비율 (0.0 ~ 1.0), 1.0을 하면 Capsule의 Radius를 기준으로 판단, 0.0이 되면 Center 점으로 판단 */
	UPROPERTY(EditAnywhere, Category = "SafeZone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RadiusRatio = 0.8f;

private:

	/** Overlapping Characters의 Set */
	TSet<TWeakObjectPtr<AUnderwaterCharacter>> OverlappingCharacters;
	
#pragma endregion
	
};
