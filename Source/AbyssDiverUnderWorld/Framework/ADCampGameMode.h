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

protected:

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void InitGameState() override;

public:

	UFUNCTION(BlueprintCallable)
	void SetSelectedLevel(const EMapName InLevelName);

	UFUNCTION(BlueprintCallable)
	void TryStartGame();

	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat")
	void GetMoney();

	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat")
	void GetSomeMoney(int32 SomeValue);

	void TravelToInGameLevel();

protected:

	// 인당 초기 지원금
	UPROPERTY(EditDefaultsOnly, Category = "ADCampGameModeSettings")
	int32 InitialSupportMoney = 300;

	uint8 bHasPressedTravel : 1 = false;

#pragma region Getter / Setter

public:

	bool HasPressedTravel() const;

#pragma endregion

};
