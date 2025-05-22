#include "Shops/ShopWidgets/ShopItemMeshPanel.h"

#include "AbyssDiverUnderWorld.h"
#include "Subsystems/DataTableSubsystem.h"
#include "DataRow/ShopItemMeshTransformRow.h"

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

void UShopItemMeshPanel::ChangeItemMesh(USkeletalMesh* NewMesh, int32 ItemId)
{
	check(ItemMeshComponent);

	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Warning, TEXT("DataTableSubsystem == nullptr"));
		return;
	}

	ItemMeshComponent->SetSkeletalMesh(NewMesh);

	if (ItemId == INDEX_NONE)
	{
		return;
	}

	FShopItemMeshTransformRow* MeshTransformData = DataTableSubsystem->GetShopItemMeshTransformData(ItemId);
	if (MeshTransformData == nullptr)
	{
		LOGV(Warning, TEXT("MeshTransformData == nullptr"));
		return;
	}

	ItemMeshComponent->SetRelativeRotation(MeshTransformData->ReletiveRotation);
	ItemMeshComponent->SetRelativeLocation(MeshTransformData->ReletivePosition);
	ItemMeshComponent->SetRelativeScale3D(MeshTransformData->ReletiveSize);
}

void UShopItemMeshPanel::SetItemMeshActive(bool bShouldActivate)
{
	check(ItemMeshComponent);
	ItemMeshComponent->SetActive(bShouldActivate);
}

void UShopItemMeshPanel::AddMeshRotationYaw(float Yaw)
{
	if (Yaw == 0.0f)
	{
		return;
	}

	FRotator NewRotator = ItemMeshComponent->GetComponentRotation();
	NewRotator.Yaw += Yaw;
	SetMeshRotation(NewRotator);
}

void UShopItemMeshPanel::SetMeshRotation(const FRotator& NewRotator)
{
	ItemMeshComponent->SetWorldRotation(NewRotator, true);
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
