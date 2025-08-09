#include "Subsystems/ADWorldSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADPlayerController.h"
#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"

#include "EngineUtils.h"

const FString UADWorldSubsystem::MainMenuLevelName = TEXT("MainLevel");
const FString UADWorldSubsystem::CampLevelName = TEXT("Submarine_Lobby");
const FString UADWorldSubsystem::ShallowLevelName = TEXT("Shallow_Test");
const FString UADWorldSubsystem::DeepLevelName = TEXT("DeepAbyss");

void UADWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	FString WorldName;
	InWorld.GetName(WorldName);
	CurrentLevelName = WorldName;
	LOGV(Log, TEXT("%s Map Has BegunPlay"), *WorldName);

	if (MainMenuLevelName == WorldName)
	{
		return;
	}

	AADPlayerController* PlayerController = InWorld.GetFirstPlayerController <AADPlayerController>();
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->SetShowMouseCursor(false);
	PlayerController->SetIgnoreMoveInput(false);
	// 카메라 페이드 아웃이 적용되어 있으면 원래대로 복구한다.
	PlayerController->ShowFadeIn();

	if (PlayerController->HasAuthority())
	{
		for (AADPlayerController* PC : TActorRange<AADPlayerController>(&InWorld))
		{
			if (AADPlayerState* ADPlayerState = PC->GetPlayerState<AADPlayerState>())
			{
				UADInventoryComponent* Inventory = ADPlayerState->GetInventory();
				if (Inventory)
				{
					Inventory->UnEquip();
					TArray<FItemData> Items = Inventory->GetInventoryList().Items;
					for (const FItemData& ItemData : Items)
					{
						if (ItemData.ItemType == EItemType::Exchangable)
						{
							Inventory->RemoveBySlotIndex(ItemData.SlotIndex, EItemType::Exchangable, false);
						}
					}
				}
			}
		}
	}
}
