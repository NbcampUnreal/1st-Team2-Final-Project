#pragma once

#include "CoreMinimal.h"
#include "Monster/Boss/EnhancedBossAIController.h"
#include "EyeStalkerAIController.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AEyeStalkerAIController : public ABossAIController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;


public:
	void InitTargetPlayer();
	void RemoveTargetPlayer(AUnderwaterCharacter* Player);

protected:
	void OnSightPerceptionSuccess(AUnderwaterCharacter* Player);
	void OnSightPerceptionFail(AUnderwaterCharacter* Player);

private:
	UFUNCTION()
	void OnSightPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus);

	void SetRandomTargetPlayer();
	void RemoveInValidTargetPlayers();

private:
	UPROPERTY()
	TArray<TObjectPtr<AUnderwaterCharacter>> TargetPlayers;

public:
	FORCEINLINE	TArray<AUnderwaterCharacter*> GetTargetPlayers() const { return TargetPlayers; }
	
};
