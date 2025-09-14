#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MissionEventHubComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMonsterKilled, FName /*UnitId*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemCollected, uint8 /*ItemId*/, int32 /*Amount*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, uint8 /*ItemId*/, int32 /*Amount*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteracted, FGameplayTag /*TargetTag*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAggroTriggered, FGameplayTag /*SourceTag*/);


UCLASS( ClassGroup=(Mission), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UMissionEventHubComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMissionEventHubComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// 게임 시스템들이 호출할 API
    void BroadcastMonsterKilled(FName UnitId);
    void BroadcastItemCollected(uint8 ItemId, int32 Amt);
    void BroadcastItemUsed(uint8 ItemId, int32 Amt);
    void BroadcastInteracted(FGameplayTag Tag);
    void BroadcastAggro(FGameplayTag SourceTag);

    // 미션 매니저가 구독할 델리게이트
    FOnMonsterKilled    OnMonsterKilled;
    FOnItemCollected    OnItemCollected;
    FOnItemUsed         OnItemUsed;
    FOnInteracted       OnInteracted;
    FOnAggroTriggered   OnAggroTriggered;
		
};
