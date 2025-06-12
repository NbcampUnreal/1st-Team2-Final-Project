#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"
#include "Character/UpgradeComponent.h"


AADPlayerState::AADPlayerState()
	: PlayerNickname(TEXT("Player"))
	, TotalPersonalCredit(0)
	, TotalMonsterKillCount(0)
	, TotalOreMinedCount(0)
	, SafeReturnCount(0)
	, PersonalCredit(0)
	, MonsterKillCount(0)
	, OreMinedCount(0)
	, bIsSafeReturn(false)
	, PlayerIndex(INDEX_NONE)
{
	bReplicates = true;


	InventoryComp = CreateDefaultSubobject<UADInventoryComponent>(TEXT("InventoryComp"));
	UpgradeComp = CreateDefaultSubobject<UUpgradeComponent>(TEXT("UpgradeComp"));
}

void AADPlayerState::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = GetPlayerController();
	if (PC && PC->IsLocalController() && HasAuthority())
	{
		LOGVN(Error, TEXT("Map Name : %s"), *GetWorld()->GetMapName());
		InventoryComp->ClientRequestInventoryInitialize();
		LOGVN(Error, TEXT("Inventory Initializded"));

	}
}

//Client
void AADPlayerState::PostNetInit()
{
	Super::PostNetInit();

	APlayerController* PC = GetPlayerController();
	if (PC && PC->IsLocalController())
	{

		InventoryComp->ClientRequestInventoryInitialize();
		LOGVN(Error, TEXT("Inventory Initializded"));
	}
}

void AADPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AADPlayerState, PlayerNickname);
	DOREPLIFETIME(AADPlayerState, TotalPersonalCredit);
	DOREPLIFETIME(AADPlayerState, TotalMonsterKillCount);
	DOREPLIFETIME(AADPlayerState, TotalOreMinedCount);
	DOREPLIFETIME(AADPlayerState, SafeReturnCount);
	DOREPLIFETIME(AADPlayerState, PersonalCredit);
	DOREPLIFETIME(AADPlayerState, MonsterKillCount);
	DOREPLIFETIME(AADPlayerState, OreMinedCount);
	DOREPLIFETIME(AADPlayerState, bIsSafeReturn);
	DOREPLIFETIME(AADPlayerState, PlayerIndex);
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

	LOGV(Warning, TEXT("Id Copied, Old : %d, New : %d, Net : %s"), PlayerIndex, NextPlayerState->GetPlayerIndex(), *UniqueNetIdRepl->ToString());
	
	NextPlayerState->SetPlayerIndex(PlayerIndex);
	if (UUpgradeComponent* NextUpgradeComponent = NextPlayerState->GetUpgradeComp())
	{
		NextUpgradeComponent->CopyProperties(GetUpgradeComp());
	}

	if (UADInventoryComponent* NextInventoryComp = NextPlayerState->GetInventory())
	{
		NextInventoryComp->CopyInventoryFrom(GetInventory());
	}

	NextPlayerState->SetPlayerNickname(PlayerNickname);
}

void AADPlayerState::SetPlayerInfo( const FString& InNickname)
{
	if (HasAuthority())
	{
		PlayerNickname = InNickname;

		OnRep_Nickname();
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

void AADPlayerState::OnRep_Nickname()
{
}
