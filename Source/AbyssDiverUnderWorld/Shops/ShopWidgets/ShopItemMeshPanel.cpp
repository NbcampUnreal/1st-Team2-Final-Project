#include "Shops/ShopWidgets/ShopItemMeshPanel.h"

#include "AbyssDiverUnderWorld.h"

FReply UShopItemMeshPanel::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bIsMouseDown = true;
	CurrentMousePositionX = InMouseEvent.GetScreenSpacePosition().X;

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
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

void UShopItemMeshPanel::AddMeshRotationYaw(float Yaw)
{
	FRotator NewRotator = ItemMeshComponent->GetComponentRotation();
	NewRotator.Yaw += Yaw;
	SetMeshRotation(NewRotator);
}

void UShopItemMeshPanel::SetMeshRotation(const FRotator& NewRotator)
{
	ItemMeshComponent->SetRelativeRotation(NewRotator.Quaternion(), true);
}

bool UShopItemMeshPanel::GetMouseDown() const
{
	return bIsMouseDown;
}

void UShopItemMeshPanel::SetMouseDown(bool bNewMouseDown)
{
	bIsMouseDown = bNewMouseDown;
}

float UShopItemMeshPanel::GetCurrentMousePositionY() const
{
	return CurrentMousePositionX;
}

void UShopItemMeshPanel::SetCurrentMousePositionX(float NewPositionX)
{
	CurrentMousePositionX = NewPositionX;
}
