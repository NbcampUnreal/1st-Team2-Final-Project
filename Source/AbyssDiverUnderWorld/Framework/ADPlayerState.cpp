#include "Framework/ADPlayerState.h"

#include "Inventory/ADInventoryComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "Character/UpgradeComponent.h"

#include "Net/UnrealNetwork.h"

AADPlayerState::AADPlayerState()
	: TotalPersonalCredit(0)
	, TotalMonsterKillCount(0)
	, TotalOreMinedCount(0)
	, SafeReturnCount(0)
	, PersonalCredit(0)
	, Damage(0)
	, MonsterKillCount(0)
	, Assists(0)
	, GroggyRevive(0)
	, CorpseRecovery(0)
	, OreMinedCount(0)
	, OreCollectedValue(0)
	, bIsSafeReturn(false)
	, PlayerIndex(INDEX_NONE)
	, bHasBeenDead(false)
	, LastOxygenRemain(0.0f)
	, bIsDead(false)
{
	bReplicates = true;

	InventoryComp = CreateDefaultSubobject<UADInventoryComponent>(TEXT("InventoryComp"));
	UpgradeComp = CreateDefaultSubobject<UUpgradeComponent>(TEXT("UpgradeComp"));
}

void AADPlayerState::BeginPlay()
{
	Super::BeginPlay();
	

	APlayerController* PC = GetPlayerController();
	if (PC == nullptr || HasAuthority() == false)
	{
		return;
	}

	if (PC->IsLocalController() == false)
	{
		return;
	}
	
	LOGVN(Log, TEXT("Map Name : %s"), *GetWorld()->GetMapName());
	InventoryComp->ClientRequestInventoryInitialize();
	LOGVN(Log, TEXT("Inventory Initialized"));
}

//Client
void AADPlayerState::PostNetInit()
{
	Super::PostNetInit();

	APlayerController* PC = GetPlayerController();
	if (PC == nullptr)
	{
		return;
	}

	if (PC->IsLocalController() == false)
	{
		return;
	}

	InventoryComp->ClientRequestInventoryInitialize();
	LOGVN(Log, TEXT("Inventory Initialized"));
}

void AADPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AADPlayerState, TotalPersonalCredit);
	DOREPLIFETIME(AADPlayerState, TotalMonsterKillCount);
	DOREPLIFETIME(AADPlayerState, TotalOreMinedCount);
	DOREPLIFETIME(AADPlayerState, SafeReturnCount);
	DOREPLIFETIME(AADPlayerState, PersonalCredit);
	DOREPLIFETIME(AADPlayerState, Damage);
	DOREPLIFETIME(AADPlayerState, MonsterKillCount);
	DOREPLIFETIME(AADPlayerState, Assists);
	DOREPLIFETIME(AADPlayerState, OreMinedCount);
	DOREPLIFETIME(AADPlayerState, OreCollectedValue);
	DOREPLIFETIME(AADPlayerState, bIsSafeReturn);
	DOREPLIFETIME(AADPlayerState, PlayerIndex);
	DOREPLIFETIME(AADPlayerState, GroggyRevive);
	DOREPLIFETIME(AADPlayerState, CorpseRecovery);
}

void AADPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (IsValid(this) == false)
	{
		LOGVN(Error, TEXT("IsValid(this) == false"));
		return;
	}

	if (IsValid(PlayerState) == false)
	{
		LOGVN(Error, TEXT("IsValid(PlayerState) == false"));
		return;
	}

	AADPlayerState* NextPlayerState = CastChecked<AADPlayerState>(PlayerState);

	const FUniqueNetIdRepl& UniqueNetIdRepl = NextPlayerState->GetUniqueId();
	if (UniqueNetIdRepl.IsValid() == false)
	{
		LOGVN(Error, TEXT("UniqueNetIdRepl.IsValid() == false"));
		return;
	}

	NextPlayerState->SetPlayerIndex(PlayerIndex);
	LOGV(Log, TEXT("Id Copied, Old : %d, New : %d, Net : %s"), PlayerIndex, NextPlayerState->GetPlayerIndex(), *UniqueNetIdRepl->ToString());

	if (UUpgradeComponent* NextUpgradeComponent = NextPlayerState->GetUpgradeComp())
	{
		NextUpgradeComponent->CopyProperties(GetUpgradeComp());
	}

	if (UADInventoryComponent* NextInventoryComp = NextPlayerState->GetInventory())
	{
		NextInventoryComp->CopyInventoryFrom(GetInventory());
	}
}

void AADPlayerState::SetPlayerInfo( const FString& InNickname)
{
	if (HasAuthority())
	{
		SetPlayerNickname(InNickname);
	}
}

void AADPlayerState::ApplyLevelResultsToTotal()
{
	if (HasAuthority())
	{
		TotalPersonalCredit += PersonalCredit;
		TotalMonsterKillCount += MonsterKillCount;
		TotalOreMinedCount += OreMinedCount;
		SafeReturnCount += bIsSafeReturn ? 1 : 0;
	}

	ResetLevelResults();
}

void AADPlayerState::ResetLevelResults()
{
	if (HasAuthority())
	{
		PersonalCredit = 0;
		MonsterKillCount = 0;
		OreMinedCount = 0;
		bIsSafeReturn = false;
	}
}

void AADPlayerState::SetPlayerNickname(const FString& NewName)
{
	SetPlayerName(NewName);
}

const FString AADPlayerState::GetPlayerNickname() const
{
	return GetPlayerName();
}

void AADPlayerState::SetOreCollectedValue(int32 Value)
{
	if (!HasAuthority()) return;

	if (OreCollectedValue != Value)
	{
		OreCollectedValue = Value;

		OnOreCollectedValueChangedDelegate.Broadcast(this, OreCollectedValue);
		LOGV(Log, TEXT("OreCollectedValue Changed: %d"), OreCollectedValue);
	}
}

void AADPlayerState::AddOreCollectedValue(int32 Value)
{
	if (!HasAuthority()) return;

	const int32 NewValue = OreCollectedValue + Value;
	SetOreCollectedValue(NewValue);
}

void AADPlayerState::SetIsDead(bool bNewIsDead)
{
	bIsDead = bNewIsDead;
}
