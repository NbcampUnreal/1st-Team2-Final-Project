#include "Boss/EQS/EnvQueryContext_ChasingTarget.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Character/UnderwaterCharacter.h"

void UEnvQueryContext_ChasingTarget::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                    FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	ABoss* Boss = Cast<ABoss>(QueryInstance.Owner);
	if (!IsValid(Boss) || !IsValid(Boss->GetTarget())) return;
	
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Boss->GetTarget());
}
