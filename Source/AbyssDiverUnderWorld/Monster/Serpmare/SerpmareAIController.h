#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterAIController.h"

#include "SerpmareAIController.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API ASerpmareAIController : public AMonsterAIController
{
	GENERATED_BODY()

public:

	ASerpmareAIController();

protected:

	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;
	
};
