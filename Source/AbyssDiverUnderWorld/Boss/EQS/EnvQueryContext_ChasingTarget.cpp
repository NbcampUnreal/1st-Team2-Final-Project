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
	if (!IsValid(Boss)) return;

	// 캐시된 타겟이 없다면 보스를 컨텍스트로 사용
	if (!IsValid(Boss->GetCachedTarget()))
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Boss);
		return;
	}

	// 캐시된 타겟이 있는 경우 타겟을 컨텍스트로 사용
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Boss->GetCachedTarget());
}
