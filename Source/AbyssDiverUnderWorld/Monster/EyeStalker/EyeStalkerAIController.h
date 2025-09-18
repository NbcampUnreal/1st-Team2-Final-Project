#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterAIController.h"

#include "EyeStalkerAIController.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AEyeStalkerAIController : public AMonsterAIController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;


public:

	void InitTargetPlayer();

};
