#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "BattleFieldVolume.generated.h"

class AUnderwaterCharacter;

DECLARE_MULTICAST_DELEGATE(FOnBattleFieldBeginOverlap);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBattleFieldEndOverlap, const uint8&);

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABattleFieldVolume : public ATriggerBox
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	FOnBattleFieldBeginOverlap OnBattleFieldBeginOverlapDelegate;
	FOnBattleFieldEndOverlap OnBattleFieldEndOverlapDelegate;

public:
	/** 현재 전투 필드에 남아있는 플레이어 중 랜덤으로 1명 반환하는 함수 */
	AUnderwaterCharacter* GetBattleFieldPlayer();

private:
	UFUNCTION()
	void OnComponentBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnComponentEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

private:
	/** 전투 필드에 남아있는 플레이어 목록 */
	TArray<TObjectPtr<AUnderwaterCharacter>> BattleFieldPlayers;
	
};
