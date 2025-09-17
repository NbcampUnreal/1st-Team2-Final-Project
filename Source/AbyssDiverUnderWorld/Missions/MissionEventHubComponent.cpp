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

void UMissionEventHubComponent::BroadcastMonsterKilled(const FGameplayTagContainer& UnitTags)
{
	if (OnMonsterKilled.IsBound())
		OnMonsterKilled.Broadcast(UnitTags);
}

void UMissionEventHubComponent::BroadcastItemCollected(const FGameplayTagContainer& ItemTags, int32 Amount)
{
	if (OnItemCollected.IsBound())
		OnItemCollected.Broadcast(ItemTags, Amount);
}

void UMissionEventHubComponent::BroadcastItemUsed(const FGameplayTagContainer& ItemTags, int32 Amount)
{
	if (OnItemUsed.IsBound())
		OnItemUsed.Broadcast(ItemTags, Amount);
}

void UMissionEventHubComponent::BroadcastAggroTriggered(const FGameplayTagContainer& SourceTags)
{
	if (OnAggroTriggered.IsBound())
		OnAggroTriggered.Broadcast(SourceTags);
}

void UMissionEventHubComponent::BroadcastInteracted(const FGameplayTagContainer& InteractTags)
{
	if (OnInteracted.IsBound())
		OnInteracted.Broadcast(InteractTags);
}


