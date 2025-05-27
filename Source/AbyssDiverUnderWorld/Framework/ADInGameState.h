#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ADInGameState.generated.h"

enum class EMapName : uint8;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStatePropertyChangedDelegate, int32 /*Changed Value*/);

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADInGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AADInGameState();

#pragma region Method
public:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostNetInit() override;

	UFUNCTION(BlueprintCallable, Exec)
	void AddTeamCredit(int32 Credit);
	UFUNCTION(BlueprintCallable, Exec)
	void IncrementPhase();

	UFUNCTION(BlueprintCallable)
	void SendDataToGameInstance();

	FString GetMapDisplayName() const;

	FOnGameStatePropertyChangedDelegate TeamCreditsChangedDelegate;
	FOnGameStatePropertyChangedDelegate CurrentPhaseChangedDelegate;
	FOnGameStatePropertyChangedDelegate CurrentPhaseGoalChangedDelegate;

protected:

	UFUNCTION()
	void OnRep_Money();

	UFUNCTION()
	void OnRep_Phase();

	UFUNCTION()
	void OnRep_PhaseGoal();

private:

	void ReceiveDataFromGameInstance();

#pragma endregion

#pragma region Variable


protected:
	UPROPERTY(Replicated)
	EMapName SelectedLevelName;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Money)
	int32 TeamCredits;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Phase, BlueprintReadOnly)
	uint8 CurrentPhase;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_PhaseGoal, BlueprintReadOnly)
	int32 CurrentPhaseGoal = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxPhase = 3;

private:

#pragma endregion

#pragma region Getter, Setteer
public:
	FORCEINLINE void SetTotalTeamCredit(int32 InMoney) 
	{ 
		if (HasAuthority() == false)
		{
			return;
		}

		TeamCredits = InMoney; 
		TeamCreditsChangedDelegate.Broadcast(InMoney);

	}

	int32 GetTotalTeamCredit() const { return TeamCredits; }

	FORCEINLINE void SetPhase(uint8 InPhase)
	{ 
		if (HasAuthority() == false)
		{
			return;
		}

		CurrentPhase = InPhase;
		CurrentPhaseChangedDelegate.Broadcast(InPhase);
	}

	uint8 GetPhase() const { return CurrentPhase; }

	uint8 GetMaxPhase() const { return MaxPhase; }

	FORCEINLINE void SetCurrentPhaseGoal(int32 NewPhaseGoal) 
	{ 
		if (HasAuthority() == false)
		{
			return;
		}

		CurrentPhaseGoal = NewPhaseGoal;
		CurrentPhaseGoalChangedDelegate.Broadcast(NewPhaseGoal);
	}

	int32 GetCurrentPhaseGoal() const { return CurrentPhaseGoal; }

	void SetSelectedLevel(EMapName LevelName) { SelectedLevelName = LevelName; }
	EMapName GetSelectedLevel() const { return SelectedLevelName; }
#pragma endregion

};
