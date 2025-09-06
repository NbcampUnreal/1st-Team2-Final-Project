#pragma once

#include "CoreMinimal.h"
#include "Monster/Monster.h"

#include "SeaObserver.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASeaObserver : public AMonster
{
	GENERATED_BODY()

public:
	ASeaObserver();

public:
	virtual void BeginPlay() override;
	virtual void OnDeath() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> BiteCollision;
	
};
