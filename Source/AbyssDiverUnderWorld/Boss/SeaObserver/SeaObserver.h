#pragma once

#include "CoreMinimal.h"
#include "Boss/Boss.h"
#include "SeaObserver.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASeaObserver : public ABoss
{
	GENERATED_BODY()

public:
	ASeaObserver();

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> BiteCollision;
	
};
