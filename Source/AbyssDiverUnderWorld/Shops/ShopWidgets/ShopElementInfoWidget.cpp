#include "ShopElementInfoWidget.h"

#include "Components/RichTextBlock.h"
#include "Components/Button.h"

void UShopElementInfoWidget::NativeConstruct()
{
	BuyButton->OnClicked.AddDynamic(this, &UShopElementInfoWidget::OnBuyButtonClicked);
}

void UShopElementInfoWidget::Init(USkeletalMeshComponent* NewItemMeshComp)
{
	ItemMeshComponent = NewItemMeshComp;
}

void UShopElementInfoWidget::ShowItemInfos(USkeletalMesh* NewItemMesh, const FString& NewDescription, const FString& NewInfoText)
{
	SetItemMeshActive(true);
	SetDescriptionActive(true);
	SetInfoTextActive(true);
	SetBuyButtonActive(true);

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

void UShopElementInfoWidget::ChangeItemMesh(USkeletalMesh* NewMesh)
{
	check(ItemMeshComponent);

	ItemMeshComponent->SetSkeletalMesh(NewMesh);
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

void UShopElementInfoWidget::SetBuyButtonActive(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		BuyButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		BuyButton->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UShopElementInfoWidget::OnBuyButtonClicked()
{
	OnBuyButtonClickedDelegate.Broadcast();
}
