#include "Monster/GobleFish/GobleFishAIController.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Monster.h"
#include "Character/UnderwaterCharacter.h"

AGobleFishAIController::AGobleFishAIController()
{
	MonsterID = "GobleFish";
}

void AGobleFishAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	Super::OnTargetPerceptionUpdated(Actor, Stimulus);

}
