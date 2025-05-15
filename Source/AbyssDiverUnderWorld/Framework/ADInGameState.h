#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ADInGameState.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADInGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AADInGameState();

#pragma region Method
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void AddMoney(int32 InMoney);
	UFUNCTION(BlueprintCallable)
	void IncrementPhase();

protected:
	

	UFUNCTION()
	void OnRep_Money();
	UFUNCTION()
	void OnRep_Phase();

	

private:
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(Replicated)
	FName SelectedLevelName;


	UPROPERTY(ReplicatedUsing = OnRep_Money)
	int32 TotalMoney;
	UPROPERTY(ReplicatedUsing = OnRep_Phase, BlueprintReadOnly)
	uint8 Phase = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxPhase = 3;

protected:


private:

#pragma endregion

#pragma region Getter, Setteer
public:
	void SetSelectedLevel(FName LevelName) { SelectedLevelName = LevelName; }
	FName GetSelectedLevel() const { return SelectedLevelName; }
#pragma endregion

};
