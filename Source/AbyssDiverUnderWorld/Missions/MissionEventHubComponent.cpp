#include "Missions/MissionEventHubComponent.h"

// Sets default values for this component's properties
UMissionEventHubComponent::UMissionEventHubComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UMissionEventHubComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UMissionEventHubComponent::BroadcastMonsterKilled(FGameplayTag UnitTag)
{
	if (OnMonsterKilled.IsBound())
	{
		OnMonsterKilled.Broadcast(UnitTag);
	}
}

void UMissionEventHubComponent::BroadcastItemCollected(FGameplayTag ItemTag, int32 Amount)
{
	if (OnItemCollected.IsBound())
	{
		OnItemCollected.Broadcast(ItemTag, Amount);
	}
}

void UMissionEventHubComponent::BroadcastItemUsed(uint8 ItemId, int32 Amt)
{
	if (OnItemUsed.IsBound())
	{
		OnItemUsed.Broadcast(ItemId, Amt);
	}
}

void UMissionEventHubComponent::BroadcastInteracted(FGameplayTag Tag)
{
	if (OnInteracted.IsBound())
	{
		OnInteracted.Broadcast(Tag);
	}
}

void UMissionEventHubComponent::BroadcastAggro(FGameplayTag SourceTag)
{
	if (OnAggroTriggered.IsBound())
	{
		OnAggroTriggered.Broadcast(SourceTag);
	}
}


