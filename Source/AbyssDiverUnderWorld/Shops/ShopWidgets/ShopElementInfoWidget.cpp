#include "ShopElementInfoWidget.h"

#include "Shops/ShopWidgets/ShopItemMeshPanel.h"

#include "Components/RichTextBlock.h"
#include "Components/Button.h"

void UShopElementInfoWidget::NativeConstruct()
{
	if (BuyButton->OnClicked.IsBound() == false)
	{
		BuyButton->OnClicked.AddDynamic(this, &UShopElementInfoWidget::OnBuyButtonClicked);
	}
}

void UShopElementInfoWidget::Init(USkeletalMeshComponent* NewItemMeshComp)
{
	ItemMeshPanel->Init(NewItemMeshComp);
}

void UShopElementInfoWidget::ShowItemInfos(USkeletalMesh* NewItemMesh, const FString& NewDescription, const FString& NewInfoText)
{
	SetItemMeshActive(true);
	SetDescriptionActive(true);
	SetInfoTextActive(true);
	SetBuyButtonActive(true);
	SetUpgradeLevelInfoActive(false);
	SetUpgradeCostInfo(false);

	ChangeItemMesh(NewItemMesh);
	ChangeItemDescription(NewDescription);
	ChangeInfoText(NewInfoText);
}

void UShopElementInfoWidget::ShowUpgradeInfos(USkeletalMesh* NewUpgradeItemMesh, int32 CurrentUpgradeLevel, bool bIsMaxLevel, int32 CurrentUpgradeCost, const FString& ExtraInfoText)
{
	SetItemMeshActive(true);
	SetDescriptionActive(false);
	SetInfoTextActive(false);
	SetBuyButtonActive(true);
	SetUpgradeLevelInfoActive(true);
	SetUpgradeCostInfo(true);

	ChangeItemMesh(NewUpgradeItemMesh);
	ChangeUpgradeLevelInfo(CurrentUpgradeLevel, bIsMaxLevel);
	ChangeUpgradeCostInfo(CurrentUpgradeCost);
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
	ItemMeshPanel->ChangeItemMesh(NewMesh);
}

void UShopElementInfoWidget::ChangeUpgradeLevelInfo(int32 CurrentLevel, bool bIsMaxLevel)
{
	FString NewInfoText = FString::FromInt(CurrentLevel);

	if (bIsMaxLevel)
	{
		NewInfoText += TEXT("Lv (Max)");
	}
	else
	{
		NewInfoText += TEXT("Lv -> ");
		NewInfoText += FString::FromInt(CurrentLevel + 1);
		NewInfoText += TEXT("Lv");
	}

	UpgradeLevelInfoText->SetText(FText::FromString(NewInfoText));
}

void UShopElementInfoWidget::ChangeUpgradeCostInfo(int32 CurrentUpgradeCost)
{
	FString NewInfoText = TEXT("업그레이드 비용 ");
	NewInfoText += FString::FromInt(CurrentUpgradeCost);
	NewInfoText += TEXT("Cr");

	UpgradeCostInfoText->SetText(FText::FromString(NewInfoText));
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
	ItemMeshPanel->SetItemMeshActive(bShouldActivate);
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

void UShopElementInfoWidget::SetUpgradeLevelInfoActive(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		UpgradeLevelInfoText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		UpgradeLevelInfoText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UShopElementInfoWidget::SetUpgradeCostInfo(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		UpgradeCostInfoText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		UpgradeCostInfoText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UShopElementInfoWidget::OnBuyButtonClicked()
{
	OnBuyButtonClickedDelegate.Broadcast();
}

UShopItemMeshPanel* UShopElementInfoWidget::GetItemMeshPanel() const
{
	return ItemMeshPanel;
}
