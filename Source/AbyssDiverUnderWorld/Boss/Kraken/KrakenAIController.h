#pragma once

#include "CoreMinimal.h"
#include "Boss/BossAIController.h"
#include "KrakenAIController.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AKrakenAIController : public ABossAIController
{
	GENERATED_BODY()

public:
	AKrakenAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

#pragma region Method
public:
	
protected:

private:
#pragma endregion

#pragma region Variable
public:

protected:

private:
#pragma endregion

#pragma region Getter, Setter
public:

#pragma endregion
	
};
