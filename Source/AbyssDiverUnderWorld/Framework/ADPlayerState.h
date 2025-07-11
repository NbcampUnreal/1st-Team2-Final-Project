#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UI/MissionData.h"
#include "OnlineSubsystem.h"
#include "ADPlayerState.generated.h"

class UADInventoryComponent;
class UUpgradeComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADPlayerState : public APlayerState
{
    GENERATED_BODY()

public:

    // 생성자
    AADPlayerState();

protected:

    // 생명주기 함수
    virtual void BeginPlay() override;
    virtual void PostNetInit() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

#pragma region Method

public:
	void SetPlayerInfo(const FString& InNickname);

	void ApplyLevelResultsToTotal();

	void ResetLevelResults();

#pragma endregion

#pragma region Variable
protected:

	UPROPERTY(Replicated)
	int32 TotalPersonalCredit;

	UPROPERTY(Replicated)
	int32 TotalMonsterKillCount;

	UPROPERTY(Replicated)
	int32 TotalOreMinedCount;

	UPROPERTY(Replicated)
	int32 SafeReturnCount;

	// Data Per Level
	UPROPERTY(Replicated)
	int32 PersonalCredit;

	UPROPERTY(Replicated)
	int32 MonsterKillCount;

	UPROPERTY(Replicated)
	int32 OreMinedCount;

	UPROPERTY(Replicated)
	uint8 bIsSafeReturn : 1;

	UPROPERTY(Replicated)
	int8 PlayerIndex;

	UPROPERTY(BlueprintReadOnly)
	uint8 bHasBeenDead : 1;

	UPROPERTY(BlueprintReadOnly)
	float LastOxygenRemain;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UADInventoryComponent> InventoryComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUpgradeComponent> UpgradeComp;

	/** 현재 사망 상태인지 여부 */
	uint8 bIsDead : 1;
	
#pragma endregion

#pragma region Getter/Setter
public:

	UFUNCTION(BlueprintCallable)
	void SetPlayerNickname(const FString& NewName);

	UFUNCTION(BlueprintPure)
	const FString GetPlayerNickname() const;

	// TotalPersonalCredit
	UFUNCTION(BlueprintCallable)
	void SetTotalPersonalCredit(int32 Value) { TotalPersonalCredit = Value; }
	UFUNCTION(BlueprintPure)
	int32 GetTotalPersonalCredit() const { return TotalPersonalCredit; }

	// TotalMonsterKillCount
	UFUNCTION(BlueprintCallable)
	void SetTotalMonsterKillCount(int32 Value) { TotalMonsterKillCount = Value; }
	UFUNCTION(BlueprintPure)
	int32 GetTotalMonsterKillCount() const { return TotalMonsterKillCount; }

	// TotalOreMinedCount
	UFUNCTION(BlueprintCallable)
	void SetTotalOreMinedCount(int32 Value) { TotalOreMinedCount = Value; }
	UFUNCTION(BlueprintPure)
	int32 GetTotalOreMinedCount() const { return TotalOreMinedCount; }

	// SafeReturnCount
	UFUNCTION(BlueprintCallable)
	void SetSafeReturnCount(int32 Value) { SafeReturnCount = Value; }
	UFUNCTION(BlueprintPure)
	int32 GetSafeReturnCount() const { return SafeReturnCount; }

	// PersonalCredit
	UFUNCTION(BlueprintCallable)
	void SetPersonalCredit(int32 Value) { PersonalCredit = Value; }
	UFUNCTION(BlueprintPure)
	int32 GetPersonalCredit() const { return PersonalCredit; }

	// MonsterKillCount
	UFUNCTION(BlueprintCallable)
	void SetMonsterKillCount(int32 Value) { MonsterKillCount = Value; }
	UFUNCTION(BlueprintPure)
	int32 GetMonsterKillCount() const { return MonsterKillCount; }

	// OreMinedCount
	UFUNCTION(BlueprintCallable)
	void SetOreMinedCount(int32 Value) { OreMinedCount = Value; }
	UFUNCTION(BlueprintPure)
	int32 GetOreMinedCount() const { return OreMinedCount; }

	// bIsSafeReturn
	UFUNCTION(BlueprintCallable)
	void SetIsSafeReturn(bool bValue) { bIsSafeReturn = bValue; }
	UFUNCTION(BlueprintPure)
	bool IsSafeReturn() const { return bIsSafeReturn; }

	FORCEINLINE UADInventoryComponent* GetInventory() const { return InventoryComp; };
	FORCEINLINE UUpgradeComponent* GetUpgradeComp() const { return UpgradeComp; };

	int8 GetPlayerIndex() const { return PlayerIndex; }
	void SetPlayerIndex(int8 NewPlayerIndex) { PlayerIndex = NewPlayerIndex; }

	FORCEINLINE void SetHasBeenDead(bool bNewDead) { bHasBeenDead = bNewDead; }
	
	FORCEINLINE bool HasBeenDead() const { return bHasBeenDead; }

	FORCEINLINE void SetLastOxygenRemain(float NewLastOxygenRemain) { LastOxygenRemain = NewLastOxygenRemain; }
	
	FORCEINLINE float GetLastOxygenRemain() const { return LastOxygenRemain; }

	FORCEINLINE bool IsDead() const { return bIsDead; }

	void SetIsDead(bool bNewIsDead);

#pragma endregion
};
