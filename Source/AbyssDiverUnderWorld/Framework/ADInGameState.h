#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ADInGameState.generated.h"

enum class EMapName : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADInGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AADInGameState();

#pragma region Method
public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Exec)
	void AddTeamCredit(int32 Credit);
	UFUNCTION(BlueprintCallable, Exec)
	void IncrementPhase();

	UFUNCTION(BlueprintCallable)
	void SendDataToGameInstance();

	FString GetMapDisplayName() const;
protected:

	UFUNCTION()
	void OnRep_Money();

	UFUNCTION()
	void OnRep_Phase();

	UFUNCTION()
	void OnRep_PhaseGoal();
	

private:
#pragma endregion

#pragma region Variable


protected:
	UPROPERTY(Replicated)
	EMapName SelectedLevelName;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Money)
	int32 TeamCredits;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Phase, BlueprintReadOnly)
	uint8 CurrentPhase;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Phase, BlueprintReadOnly)
	int32 CurrentPhaseGoal = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxPhase = 3;

private:

#pragma endregion

#pragma region Getter, Setteer
public:
	void SetTotalTeamCredit(int32 InMoney) { TeamCredits = InMoney; }
	int32 GetTotalTeamCredit() const { return TeamCredits; }

	void SetPhase(uint8 InPhase) { CurrentPhase = InPhase; }
	uint8 GetPhase() const { return CurrentPhase; }

	void SetSelectedLevel(EMapName LevelName) { SelectedLevelName = LevelName; }
	EMapName GetSelectedLevel() const { return SelectedLevelName; }
#pragma endregion

};
