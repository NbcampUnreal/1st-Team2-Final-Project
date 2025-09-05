#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterAIController.h"

#include "GobleFishAIController.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API AGobleFishAIController : public AMonsterAIController
{
	GENERATED_BODY()
	
public:

	AGobleFishAIController();

protected:

	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override; // Perception Callback Method
};
