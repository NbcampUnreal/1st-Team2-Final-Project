#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TestGameState.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATestGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ATestGameState();

#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	void AddMoney(int32 InMoney);
	UFUNCTION(BlueprintCallable)
	void IncrementPhase();

protected:
	UFUNCTION()
	void OnRep_Money();
	UFUNCTION()
	void OnRep_Phase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
#pragma endregion

#pragma region Variable
public:
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

#pragma endregion

};
