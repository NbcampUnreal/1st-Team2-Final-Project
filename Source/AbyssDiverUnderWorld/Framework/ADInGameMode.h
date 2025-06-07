#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ADInGameMode.generated.h"

enum class EMapName : uint8;
enum class ECharacterState : uint8;

class AGenericPool;
class USoundSubsystem;
class AUnderwaterCharacter;

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

	void BindDelegate(AUnderwaterCharacter* PlayerCharacter);

protected:

	void InitPlayer(APlayerController* PC);

private:

	void GameOver();

	UFUNCTION()
	void OnCharacterStateChanged(ECharacterState OldCharacterState, ECharacterState NewCharacterState);

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
	UPROPERTY()
	TObjectPtr<AGenericPool> SpearGunBulletPool = nullptr;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;

	int32 DeathCount = 0;
	int32 GroggyCount = 0;

	FTimerHandle ResultTimerHandle;

#pragma endregion

public:
	FORCEINLINE AGenericPool* GetGenericPool() const { return SpearGunBulletPool; }
	USoundSubsystem* GetSoundSubsystem();
};
