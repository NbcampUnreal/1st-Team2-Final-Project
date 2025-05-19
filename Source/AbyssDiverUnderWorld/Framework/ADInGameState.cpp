#include "Framework/ADInGameState.h"
#include "ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "AbyssDiverUnderWorld.h"
#include "DataRow/PhaseGoalRow.h"
#include "ADPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory/ADInventoryComponent.h"

AADInGameState::AADInGameState()
	: SelectedLevelName(EMapName::Max)
	, TeamCredits(0)
	, CurrentPhase(1)
	, MaxPhase(3)
{
	bReplicates = true;
	
}

void AADInGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		if (UADGameInstance* ADGameInstance = GetGameInstance<UADGameInstance>())
		{
			SelectedLevelName = ADGameInstance->SelectedLevelName;
			TeamCredits = ADGameInstance->TeamCredits;
			if (const FPhaseGoalRow* GoalData = ADGameInstance->GetSubsystem<UDataTableSubsystem>()->GetPhaseGoalData(SelectedLevelName, CurrentPhase))
			{
				CurrentPhaseGoal = GoalData->GoalCredit;
			}
		}

		LOGVN(Error, TEXT("SelectedLevelName: %d / TeamCredits: %d / CurrentPhaseGoal : %d"), SelectedLevelName, TeamCredits, CurrentPhaseGoal);
	}
}

void AADInGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		return;
	}

	TeamCreditsChangedDelegate.Broadcast(TeamCredits);
	CurrentPhaseChangedDelegate.Broadcast(CurrentPhase);
	CurrentPhaseGoalChangedDelegate.Broadcast(CurrentPhaseGoal);

}

void AADInGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AADInGameState, SelectedLevelName);
	DOREPLIFETIME(AADInGameState, TeamCredits);
	DOREPLIFETIME(AADInGameState, CurrentPhase);
}

void AADInGameState::PostNetInit()
{
	Super::PostNetInit();

	TeamCreditsChangedDelegate.Broadcast(TeamCredits);
	CurrentPhaseChangedDelegate.Broadcast(CurrentPhase);
	CurrentPhaseGoalChangedDelegate.Broadcast(CurrentPhaseGoal);
}

void AADInGameState::AddTeamCredit(int32 Credit)
{
	if (!HasAuthority()) return;

	TeamCredits += Credit;
	OnRep_Money();
}

void AADInGameState::IncrementPhase()
{
	if (!HasAuthority()) return;

	CurrentPhase = FMath::Clamp(CurrentPhase + 1, 0, MaxPhase);
	OnRep_Phase();
}

void AADInGameState::SendDataToGameInstance()
{
	if (UADGameInstance* ADGameInstance = GetGameInstance<UADGameInstance>())
	{
		ADGameInstance->SelectedLevelName = SelectedLevelName;
		ADGameInstance->TeamCredits = TeamCredits;
	}

	LOGVN(Error, TEXT("SelectedLevelName: %s / TeamCredits: %d"), SelectedLevelName, TeamCredits);
}

void AADInGameState::OnRep_Money()
{
	// UI 업데이트
	UE_LOG(LogTemp, Log, TEXT("TotalTeamCredit updated: %d"), TeamCredits);
	TeamCreditsChangedDelegate.Broadcast(TeamCredits);
}

void AADInGameState::OnRep_Phase()
{
	// UI 업데이트
	UE_LOG(LogTemp, Log, TEXT("Phase updated: %d/%d"), CurrentPhase, MaxPhase);
	CurrentPhaseChangedDelegate.Broadcast(CurrentPhase);
}

void AADInGameState::OnRep_PhaseGoal()
{
	LOGVN(Error, TEXT("PhaseGoal updated: %d"), CurrentPhaseGoal);
	CurrentPhaseGoalChangedDelegate.Broadcast(CurrentPhaseGoal);
}

FString AADInGameState::GetMapDisplayName() const
{
	const UEnum* EnumPtr = StaticEnum<EMapName>();
	if (EnumPtr == nullptr)
	{
		return TEXT("INVALID");
	}

	FString DisplayName = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(SelectedLevelName)).ToString();
	LOGV(Warning, TEXT("Display Name : %s"), *DisplayName);

	return DisplayName;

}
