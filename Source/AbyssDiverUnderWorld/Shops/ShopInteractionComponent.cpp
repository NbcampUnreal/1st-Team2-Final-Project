#include "Shops/ShopInteractionComponent.h"

#include "AbyssDiverUnderWorld.h"
#include "Character/UnderwaterCharacter.h"
#include "Shops/Shop.h"

UShopInteractionComponent::UShopInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UShopInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UShopInteractionComponent::C_OpenShop_Implementation(AShop* InteractingShop)
{
	AUnderwaterCharacter* OwningCharacter = Cast<AUnderwaterCharacter>(GetOwner());
	if (OwningCharacter == nullptr)
	{
		LOGV(Warning, TEXT("OwningCharacter == nullptr"));
		return;
	}

	SetCurrentInteractingShop(InteractingShop);
	InteractingShop->OpenShop(OwningCharacter);
}

void UShopInteractionComponent::S_RequestBuyItem_Implementation(uint8 ItemId, uint8 Quantity)
{
	if (CurrentInteractingShop == nullptr)
	{
		LOGV(Warning, TEXT("CurrentInteractingShop == nullptr"));
		return;
	}

	CurrentInteractingShop->BuyItem(ItemId, Quantity, Cast<AUnderwaterCharacter>(GetOwner()));
}

AShop* UShopInteractionComponent::GetCurrentInteractingShop() const
{
	return CurrentInteractingShop;
}

void UShopInteractionComponent::SetCurrentInteractingShop(AShop* NewInteractingShop)
{
	CurrentInteractingShop = NewInteractingShop;
}
