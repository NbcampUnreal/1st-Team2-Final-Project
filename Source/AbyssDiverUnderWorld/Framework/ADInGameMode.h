#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ADInGameMode.generated.h"

enum class EMapName : uint8;
class AGenericPool;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADInGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AADInGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

#pragma region Methods

public:

	void ReadyForTravelToCamp();
	void TravelToCamp();

	bool IsAllPhaseCleared();

protected:
	void InitPlayer(APlayerController* PC);

private:

	UFUNCTION(Exec, Category = "Cheat")
	void GetOre();
	
	UFUNCTION(Exec, Category = "Cheat")
	void GetMoney();

#pragma endregion

#pragma region Variables
private:

	UPROPERTY(EditAnywhere, Category = "InGameMode")
	FString CampMapName;

	UPROPERTY()
	TObjectPtr<class AADDrone> LastDrone;

	TSubclassOf<class AADSpearGunBullet> BulletClass;
	TObjectPtr<AGenericPool> SpearGunBulletPool = nullptr;

#pragma endregion

public:
	FORCEINLINE AGenericPool* GetGenericPool() const { return SpearGunBulletPool; }
};
