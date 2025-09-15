#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MissionEventHubComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMonsterKilled, FGameplayTag /*UnitTag*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemCollected, FGameplayTag /*ItemTag*/, int32 /*Amount*/);
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
	// ���� �ý��۵��� ȣ���� API
    void BroadcastMonsterKilled(FGameplayTag UnitTag);
    void BroadcastItemCollected(FGameplayTag ItemTag, int32 Amount);
    void BroadcastItemUsed(uint8 ItemId, int32 Amount);
    void BroadcastInteracted(FGameplayTag Tag);
    void BroadcastAggro(FGameplayTag SourceTag);

    // �̼� �Ŵ����� ������ ��������Ʈ
    FOnMonsterKilled    OnMonsterKilled;
    FOnItemCollected    OnItemCollected;
    FOnItemUsed         OnItemUsed;
    FOnInteracted       OnInteracted;
    FOnAggroTriggered   OnAggroTriggered;
		
};
