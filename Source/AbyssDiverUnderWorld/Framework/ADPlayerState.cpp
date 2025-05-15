
#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"

AADPlayerState::AADPlayerState()
{
	InventoryComp = CreateDefaultSubobject<UADInventoryComponent>(TEXT("InventoryComp"));
}



void AADPlayerState::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetPlayerController();
	if (PC && PC->IsLocalController() && HasAuthority())
	{

		InventoryComp->InventoryInitialize();
		LOGVN(Error, TEXT("Inventory Initializded"));
	}

}

void AADPlayerState::PostNetInit()
{
	Super::PostNetInit();

	APlayerController* PC = GetPlayerController();
	if (PC && PC->IsLocalController())
	{

		InventoryComp->InventoryInitialize();
		LOGVN(Error, TEXT("Inventory Initializded"));
	}
}

void AADPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AADPlayerState, ADPlayerID);
	DOREPLIFETIME(AADPlayerState, PlayerNickname);
}

void AADPlayerState::SetPlayerInfo(const FUniqueNetIdRepl& InId, const FString& InNickname)
{
	if (HasAuthority())
	{
		ADPlayerID = InId;
		PlayerNickname = InNickname;

		if (ADPlayerID.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Set PlayerInfo: ID = %s, Nickname = %s"),
				*ADPlayerID->ToString(), *PlayerNickname);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Set PlayerInfo: Invalid ID, Nickname = %s"),
				*PlayerNickname);
		}


		OnRep_Nickname();
	}
}

void AADPlayerState::OnRep_Nickname()
{
}
