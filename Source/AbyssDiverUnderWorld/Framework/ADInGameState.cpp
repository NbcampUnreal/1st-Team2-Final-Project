#include "Framework/ADInGameState.h"
#include "Net/UnrealNetwork.h"

AADInGameState::AADInGameState()
	: TotalMoney(0)
	, Phase(0)
{
	bReplicates = true;
}

void AADInGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AADInGameState, TotalMoney);
	DOREPLIFETIME(AADInGameState, Phase);
}

void AADInGameState::AddMoney(int32 InMoney)
{
	if (!HasAuthority()) return;

	TotalMoney += InMoney;
	OnRep_Money();
}

void AADInGameState::IncrementPhase()
{
	if (!HasAuthority()) return;

	Phase = FMath::Clamp(Phase + 1, 0, MaxPhase);
	OnRep_Phase();
}

void AADInGameState::OnRep_Money()
{
	// UI 업데이트
	UE_LOG(LogTemp, Log, TEXT("TotalMoney updated: %d"), TotalMoney);
}

void AADInGameState::OnRep_Phase()
{
	// UI 업데이트
	UE_LOG(LogTemp, Log, TEXT("Phase updated: %d/%d"), Phase, MaxPhase);
}
