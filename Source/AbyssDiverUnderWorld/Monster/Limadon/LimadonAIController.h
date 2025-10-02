#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterAIController.h"

#include "LimadonAIController.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ALimadonAIController : public AMonsterAIController
{
	GENERATED_BODY()

public:

	ALimadonAIController();

protected:

	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;
	
};
