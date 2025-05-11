#include "Framework/TestGameState.h"
#include "Net/UnrealNetwork.h"

ATestGameState::ATestGameState()
{
	TotalMoney = 0;
	bReplicates = true;
}

void ATestGameState::AddMoney(int32 InMoney)
{
	if (!HasAuthority()) return;

	TotalMoney += InMoney;

	OnRep_Money();
}

void ATestGameState::IncrementPhase()
{
	if (!HasAuthority()) return;

	Phase = FMath::Clamp(Phase + 1, 0, MaxPhase);
	OnRep_Phase();
}

void ATestGameState::OnRep_Money()
{
	// UI 업데이트
	UE_LOG(LogTemp, Log, TEXT("TotalMoney updated: %d"), TotalMoney);
}

void ATestGameState::OnRep_Phase()
{
	// UI 업데이트
	UE_LOG(LogTemp, Log, TEXT("Phase updated: %d/%d"), Phase, MaxPhase);
}

void ATestGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATestGameState, TotalMoney);
	DOREPLIFETIME(ATestGameState, Phase);
}
