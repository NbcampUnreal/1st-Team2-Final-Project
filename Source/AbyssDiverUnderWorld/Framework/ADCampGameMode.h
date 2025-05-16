#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ADCampGameMode.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADCampGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	void ADCampGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void TryStartGame();

protected:
	void TravelToInGameLevel();

};
