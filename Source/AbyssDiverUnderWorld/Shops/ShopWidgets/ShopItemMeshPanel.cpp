#include "Shops/ShopWidgets/ShopItemMeshPanel.h"

#include "AbyssDiverUnderWorld.h"

FReply UShopItemMeshPanel::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return Reply;
}

FReply UShopItemMeshPanel::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UShopItemMeshPanel::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void UShopItemMeshPanel::Init(USkeletalMeshComponent* NewItemMeshComp)
{
	ItemMeshComponent = NewItemMeshComp;
}

void UShopItemMeshPanel::ChangeItemMesh(USkeletalMesh* NewMesh)
{
	check(ItemMeshComponent);
	ItemMeshComponent->SetSkeletalMesh(NewMesh);
}

void UShopItemMeshPanel::SetItemMeshActive(bool bShouldActivate)
{
	check(ItemMeshComponent);
	ItemMeshComponent->SetActive(bShouldActivate);
}
