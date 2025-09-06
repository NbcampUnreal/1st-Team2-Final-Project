#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MissionEventHubComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMonsterKilled, FName /*UnitId*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemCollected, uint8 /*ItemId*/, int32 /*Amount*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, uint8 /*ItemId*/, int32 /*Amount*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteracted, FGameplayTag /*TargetTag*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAggroTriggered, FGameplayTag /*AggroTag*/);


UCLASS( ClassGroup=(Mission), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UMissionEventHubComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMissionEventHubComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// ���� �ý��۵��� ȣ���� API
    void BroadcastMonsterKilled(FName UnitId)            { OnMonsterKilled.Broadcast(UnitId); }
    void BroadcastItemCollected(uint8 ItemId, int32 Amt) { OnItemCollected.Broadcast(ItemId, Amt); }
    void BroadcastItemUsed(uint8 ItemId, int32 Amt)      { OnItemUsed.Broadcast(ItemId, Amt); }
    void BroadcastInteracted(FGameplayTag Tag)           { OnInteracted.Broadcast(Tag); }
    void BroadcastAggro(FGameplayTag Tag)                { OnAggroTriggered.Broadcast(Tag); }

    // �̼� �Ŵ����� ������ ��������Ʈ
    FOnMonsterKilled    OnMonsterKilled;
    FOnItemCollected    OnItemCollected;
    FOnItemUsed         OnItemUsed;
    FOnInteracted       OnInteracted;
    FOnAggroTriggered   OnAggroTriggered;
		
};
