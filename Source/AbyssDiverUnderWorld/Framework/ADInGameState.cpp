#include "Framework/ADInGameState.h"
#include "ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "AbyssDiverUnderWorld.h"
#include "DataRow/PhaseGoalRow.h"

AADInGameState::AADInGameState()
	: SelectedLevelName(EMapName::Max)
	, TeamCredits(0)
	, CurrentPhase(0)
	, MaxPhase(3)
{
	bReplicates = true;
	
}

void AADInGameState::BeginPlay()
{
	Super::BeginPlay();

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

void AADInGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AADInGameState, SelectedLevelName);
	DOREPLIFETIME(AADInGameState, TeamCredits);
	DOREPLIFETIME(AADInGameState, CurrentPhase);
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
}

void AADInGameState::OnRep_Phase()
{
	// UI 업데이트
	UE_LOG(LogTemp, Log, TEXT("Phase updated: %d/%d"), CurrentPhase, MaxPhase);
}

void AADInGameState::OnRep_PhaseGoal()
{
	LOGVN(Error, TEXT("PhaseGoal updated: %d"), CurrentPhaseGoal);
}

FString AADInGameState::GetMapDisplayName() const
{
	const UEnum* EnumPtr = StaticEnum<EMapName>();
	
	return EnumPtr ? EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(SelectedLevelName)).ToString()
		: TEXT("invalid");

}
