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

private:

	/** Overlapping Characters의 Set */
	TSet<TWeakObjectPtr<AUnderwaterCharacter>> OverlappingCharacters;
	
#pragma endregion
	
};
