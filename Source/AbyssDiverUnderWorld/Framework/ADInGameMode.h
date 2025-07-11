#pragma once

#include "CoreMinimal.h"
#include "ADPlayerController.h"
#include "GameFramework/GameMode.h"
#include "ADInGameMode.generated.h"

enum class EMapName : uint8;
enum class ECharacterState : uint8;

class AGenericPool;
class USoundSubsystem;
class AUnderwaterCharacter;
class AADDrone;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADInGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	AADInGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void StartPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

protected:

	enum class EPlayerAliveInfo
	{
		Alive,
		Dead,
		Absent,
		MAX
	};

#pragma region Methods

public:

	void ReadyForTravelToCamp();
	void TravelToCamp();

	bool IsAllPhaseCleared();

	void BindDelegate(AUnderwaterCharacter* PlayerCharacter);

	/** Spawn Center 중심으로 지정된 거리 내에서 랜덤한 위치를 선택하여 플레이어들을 부활시킨다. */
	void RevivePlayersAtRandomLocation(TArray<int8> PlayerIndexes, const FVector& SpawnCenter, float ReviveDistance);
	void RevivePlayersAroundDroneAtRespawnLocation(const TArray<int8>& PlayerIndexes, const AADDrone* SomeDrone);

	/** 지정된 인덱스의 플레이어 컨트롤러를 찾는다. */
	AADPlayerController* FindPlayerControllerFromIndex(int8 PlayerIndex) const;
	
protected:

	void InitPlayer(APlayerController* PC);

	/** 지정된 위치에서 지정된 거리 내에서 랜덤한 위치를 반환한다. */
	FVector GetRandomLocation(const FVector& Location, float Distance) const;
	
	void RestartPlayerFromPlayerIndex(int8 PlayerIndex, const FVector& SpawnLocation);

private:

	void GameOver();

	UFUNCTION()
	void OnCharacterStateChanged(AUnderwaterCharacter* Character, ECharacterState OldCharacterState, ECharacterState NewCharacterState);

	UFUNCTION(Exec, Category = "Cheat")
	void GetOre();
	
	UFUNCTION(Exec, Category = "Cheat")
	void GetMoney();

	UFUNCTION(Exec, Category = "Cheat")
	void GetSomeMoney(int32 SomeValue);

#pragma endregion

#pragma region Variables
private:

	UPROPERTY(EditAnywhere, Category = "InGameMode")
	FString CampMapName;

	UPROPERTY()
	TObjectPtr<class AADDrone> LastDrone;

	TSubclassOf<class AADSpearGunBullet> SpearBulletClass;
	TSubclassOf<class AADFlareGunBullet> FlareBulletClass;
	UPROPERTY()
	TObjectPtr<AGenericPool> SpearGunBulletPool = nullptr;
	UPROPERTY()
	TObjectPtr<AGenericPool> FlareGunBulletPool = nullptr;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;

	FTimerHandle ResultTimerHandle;
	FTimerHandle SyncTimerHandle;

	TArray<EPlayerAliveInfo> PlayerAliveInfos;

#pragma endregion

public:
	FORCEINLINE AGenericPool* GetSpearGenericPool() const { return SpearGunBulletPool; }
	FORCEINLINE AGenericPool* GetFlareGenericPool() const { return FlareGunBulletPool; }
	USoundSubsystem* GetSoundSubsystem();
};
