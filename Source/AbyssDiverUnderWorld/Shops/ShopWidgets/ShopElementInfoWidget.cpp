#include "ShopElementInfoWidget.h"

#include "Components/RichTextBlock.h"

void UShopElementInfoWidget::Init(UStaticMeshComponent* NewItemMeshComp)
{
	ItemMeshComponent = NewItemMeshComp;
}

void UShopElementInfoWidget::ShowItemInfos(UStaticMesh* NewItemMesh, const FString& NewDescription, const FString& NewInfoText)
{
	SetItemMeshActive(true);
	SetDescriptionActive(true);
	SetInfoTextActive(true);

	ChangeItemMesh(NewItemMesh);
	ChangeItemDescription(NewDescription);
	ChangeInfoText(NewInfoText);
}

void UShopElementInfoWidget::ChangeItemDescription(const FString& NewDescription)
{
	DescriptionText->SetText(FText::FromString(NewDescription));
}

void UShopElementInfoWidget::ChangeInfoText(const FString& NewInfoText)
{
	InfoText->SetText(FText::FromString(NewInfoText));
}

void UShopElementInfoWidget::ChangeItemMesh(UStaticMesh* NewMesh)
{
	check(ItemMeshComponent);

	ItemMeshComponent->SetStaticMesh(NewMesh);
}

void UShopElementInfoWidget::SetDescriptionActive(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		DescriptionText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		DescriptionText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UShopElementInfoWidget::SetInfoTextActive(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		InfoText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		InfoText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UShopElementInfoWidget::SetItemMeshActive(bool bShouldActivate)
{
	check(ItemMeshComponent);
	ItemMeshComponent->SetActive(bShouldActivate);
}