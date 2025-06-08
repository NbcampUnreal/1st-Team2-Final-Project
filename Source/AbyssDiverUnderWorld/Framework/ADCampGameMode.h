#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ADCampGameMode.generated.h"


enum class EMapName : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADCampGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	void ADCampGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable)
	void SetSelectedLevel(const EMapName InLevelName);

	UFUNCTION(BlueprintCallable)
	void TryStartGame();

	UFUNCTION(Exec, Category = "Cheat")
	void GetMoney();

	void TravelToInGameLevel();

};
