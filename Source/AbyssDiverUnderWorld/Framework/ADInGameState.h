#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "Net/Serialization/FastArraySerializer.h"

#include "ADInGameState.generated.h"

enum class EMapName : uint8;
enum class EMissionType : uint8;

class AADDroneSeller;

struct FActivatedMissionInfoList;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStatePropertyChangedDelegate, int32 /*Changed Value*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMissionInfosChangedDelegate, int32 /*Changed Index*/, const FActivatedMissionInfoList& /*Changed Value*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMissionInfosRemovedDelegate, int32 /*Changed Index*/, const FActivatedMissionInfoList& /*Changed Value*/);
DECLARE_MULTICAST_DELEGATE(FOnMissionListRefreshedDelegate);

#pragma region FastArraySerializer

USTRUCT(BlueprintType)
struct FActivatedMissionInfo : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	EMissionType MissionType;

	UPROPERTY()
	uint8 MissionIndex;

	UPROPERTY()
	uint8 bIsCompleted : 1;

	UPROPERTY()
	uint8 CurrentProgress;

	void PostReplicatedAdd(const FActivatedMissionInfoList& InArraySerializer);

	void PostReplicatedChange(const FActivatedMissionInfoList& InArraySerializer);

	void PreReplicatedRemove(const FActivatedMissionInfoList& InArraySerializer);

	bool operator==(const FActivatedMissionInfo& Other) const
	{
		return MissionType == Other.MissionType && MissionIndex == Other.MissionIndex;
	}
};

USTRUCT(BlueprintType)
struct FActivatedMissionInfoList : public FFastArraySerializer
{
	GENERATED_BODY()

public:

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);

	void Add(const EMissionType& MissionType, const uint8& MissionIndex);
	void Remove(const EMissionType& MissionType, const uint8& MissionIndex);
	void ModifyProgress(const EMissionType& MissionType, const uint8& MissionIndex, const uint8& NewProgress);
	void AddOrModify(const EMissionType& MissionType, const uint8& MissionIndex, const uint8& NewProgress);

	// 인덱스 반환, 없으면 INDEX_NONE 반환
	int32 Contains(const EMissionType& MissionType, const uint8& MissionIndex);
	
	void Clear(const int32 SlackCount = 0);

	FOnMissionInfosChangedDelegate OnMissionInfosChangedDelegate;
	FOnMissionInfosRemovedDelegate OnMissionInfosRemovedDelegate;
	
public:

	UPROPERTY()
	TArray<FActivatedMissionInfo> MissionInfoList;
};

template<>
struct TStructOpsTypeTraits<FActivatedMissionInfoList> : public TStructOpsTypeTraitsBase2<FActivatedMissionInfoList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

#pragma endregion

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADInGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AADInGameState();

protected:

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostNetInit() override;

#pragma region Method

public:

	UFUNCTION(BlueprintCallable)
	void AddTeamCredit(int32 Credit);

	UFUNCTION(BlueprintCallable)
	void IncrementPhase();

	UFUNCTION(BlueprintCallable)
	void SendDataToGameInstance();

	FString GetMapDisplayName() const;

	void RefreshActivatedMissionList();

	FOnGameStatePropertyChangedDelegate TeamCreditsChangedDelegate;
	FOnGameStatePropertyChangedDelegate CurrentPhaseChangedDelegate;
	FOnGameStatePropertyChangedDelegate CurrentPhaseGoalChangedDelegate;
	FOnMissionListRefreshedDelegate OnMissionListRefreshedDelegate;

protected:

	UFUNCTION()
	void OnRep_Money();

	UFUNCTION()
	void OnRep_Phase();

	UFUNCTION()
	void OnRep_PhaseGoal();

	UFUNCTION()
	void OnRep_CurrentDroneSeller();

	UFUNCTION()
	void OnRep_DestinationTarget();

private:

	void ReceiveDataFromGameInstance();

#pragma endregion

#pragma region Variable


protected:
	UPROPERTY(Replicated)
	EMapName SelectedLevelName;

	UPROPERTY(ReplicatedUsing = OnRep_Money)
	int32 TeamCredits;

	UPROPERTY(ReplicatedUsing = OnRep_Phase, BlueprintReadOnly)
	uint8 CurrentPhase;

	UPROPERTY(ReplicatedUsing = OnRep_PhaseGoal, BlueprintReadOnly)
	int32 CurrentPhaseGoal = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxPhase = 3;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentDroneSeller)
	TObjectPtr<AADDroneSeller> CurrentDroneSeller;

	UPROPERTY(ReplicatedUsing = OnRep_DestinationTarget)
	TObjectPtr<AActor> DestinationTarget;

	UPROPERTY(Replicated)
	FActivatedMissionInfoList ActivatedMissionList;

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

	UFUNCTION(BlueprintPure, Category = "ADInGameState")
	AADDroneSeller* GetCurrentDroneSeller() const { return CurrentDroneSeller; }
	FORCEINLINE void SetCurrentDroneSeller(AADDroneSeller* NewDroneSeller)
	{
		if (HasAuthority() == false)
		{
			return;
		}

		CurrentDroneSeller = NewDroneSeller;
		OnRep_CurrentDroneSeller();
	}

	UFUNCTION(BlueprintPure, Category = "ADInGameState")
	AActor* GetDestinationTarget() const { return DestinationTarget; }
	void SetDestinationTarget(AActor* NewDestinationTarget);

	FActivatedMissionInfoList& GetActivatedMissionList() { return ActivatedMissionList; }

#pragma endregion

};
