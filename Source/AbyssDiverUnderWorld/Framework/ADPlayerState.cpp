#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "Character/UpgradeComponent.h"

AADPlayerState::AADPlayerState()
{
	InventoryComp = CreateDefaultSubobject<UADInventoryComponent>(TEXT("InventoryComp"));
	UpgradeComp = CreateDefaultSubobject<UUpgradeComponent>(TEXT("UpgradeComp"));
}

void AADPlayerState::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetPlayerController();
	if (PC && PC->IsLocalController() && HasAuthority())
	{

		InventoryComp->InventoryInitialize();
		LOGVN(Log, TEXT("Inventory Initializded"));
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
