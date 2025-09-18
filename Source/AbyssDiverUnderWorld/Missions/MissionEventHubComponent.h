#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MissionEventHubComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMonsterKilled, const FGameplayTagContainer& /*UnitTag*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemCollected, const FGameplayTagContainer& /*ItemTag*/, int32 /*Amount*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, const FGameplayTagContainer&, int32 /*Amount*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteracted, const FGameplayTagContainer& /*TargetTag*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAggroTriggered, const FGameplayTagContainer& /*SourceTag*/);


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
    UFUNCTION(BlueprintCallable) void BroadcastMonsterKilled(const FGameplayTagContainer& UnitTags);
    UFUNCTION(BlueprintCallable) void BroadcastItemCollected(const FGameplayTagContainer& ItemTags, int32 Amount);
    UFUNCTION(BlueprintCallable) void BroadcastItemUsed(const FGameplayTagContainer& ItemTags, int32 Amount);
    UFUNCTION(BlueprintCallable) void BroadcastAggroTriggered(const FGameplayTagContainer& SourceTags);
    UFUNCTION(BlueprintCallable) void BroadcastInteracted(const FGameplayTagContainer& InteractTags);

    // 미션 매니저가 구독할 델리게이트
    FOnMonsterKilled    OnMonsterKilled;
    FOnItemCollected    OnItemCollected;
    FOnItemUsed         OnItemUsed;
    FOnInteracted       OnInteracted;
    FOnAggroTriggered   OnAggroTriggered;
		
};
