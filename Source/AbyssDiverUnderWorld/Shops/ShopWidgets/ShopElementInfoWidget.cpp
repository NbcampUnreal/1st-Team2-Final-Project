#include "ShopElementInfoWidget.h"

#include "Shops/ShopWidgets/ShopItemMeshPanel.h"

#include "Components/RichTextBlock.h"
#include "Components/Button.h"
#include "Components/Overlay.h"

void UShopElementInfoWidget::NativeConstruct()
{
	if (BuyButton->OnClicked.IsBound() == false)
	{
		BuyButton->OnClicked.AddDynamic(this, &UShopElementInfoWidget::OnBuyButtonClicked);
	}

	if (IncreaseButton->OnClicked.IsBound() == false)
	{
		IncreaseButton->OnClicked.AddDynamic(this, &UShopElementInfoWidget::OnIncreaseButtonClicked);
	}

	if (DecreaseButton->OnClicked.IsBound() == false)
	{
		DecreaseButton->OnClicked.AddDynamic(this, &UShopElementInfoWidget::OnDecreaseButtonClicked);
	}
}

void UShopElementInfoWidget::Init(USkeletalMeshComponent* NewItemMeshComp)
{
	ItemMeshPanel->Init(NewItemMeshComp);
	CurrentQuantityNumber = 0;
	ChangeCurrentQuantityNumber(0);
}

void UShopElementInfoWidget::ShowItemInfos(USkeletalMesh* NewItemMesh, const FString& NewDescription, const FString& NewNameInfoText)
{
	SetItemMeshActive(true);
	SetDescriptionActive(true);
	SetNameInfoTextActive(true);
	SetBuyButtonActive(true);
	SetUpgradeLevelInfoActive(false);
	SetCostInfoActive(false);

	ChangeItemMesh(NewItemMesh);
	ChangeItemDescription(NewDescription);
	ChangeNameInfoText(NewNameInfoText);
	ChangeCurrentQuantityNumber(1);
	SetQuantityOverlayActive(true);
}

void UShopElementInfoWidget::ShowUpgradeInfos(USkeletalMesh* NewUpgradeItemMesh, int32 CurrentUpgradeLevel, bool bIsMaxLevel, int32 CurrentUpgradeCost, const FString& ExtraInfoText)
{
	SetItemMeshActive(true);
	SetDescriptionActive(false);
	SetNameInfoTextActive(false);
	SetBuyButtonActive(true);
	SetUpgradeLevelInfoActive(true);
	SetCostInfoActive(true);
	SetQuantityOverlayActive(false);

	ChangeItemMesh(NewUpgradeItemMesh);
	ChangeUpgradeLevelInfo(CurrentUpgradeLevel, bIsMaxLevel);
	ChangeCostInfo(CurrentUpgradeCost, true);
	ChangeCurrentQuantityNumber(1);
}

void UShopElementInfoWidget::ChangeItemDescription(const FString& NewDescription)
{
	DescriptionText->SetText(FText::FromString(NewDescription));
}

void UShopElementInfoWidget::ChangeNameInfoText(const FString& NewInfoText)
{
	NameInfoText->SetText(FText::FromString(NewInfoText));
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

void UShopElementInfoWidget::ChangeCostInfo(int32 CurrentCost, bool bIsUpgradeCost)
{
	FString NewInfoText = "";

	if (bIsUpgradeCost)
	{
		NewInfoText = TEXT("<S>업그레이드 비용 ");
	}
	else
	{
		NewInfoText = TEXT("<S>비용 ");
	}

	NewInfoText += FString::FromInt(CurrentCost);
	NewInfoText += TEXT("Cr</>");
	
	CostInfoText->SetText(FText::FromString(NewInfoText));
}

void UShopElementInfoWidget::ChangeCurrentQuantityNumber(int32 NewNumber)
{
	CurrentQuantityNumber = FMath::Clamp(NewNumber, 1, MAX_ITEM_COUNT);
	CurrentQuantityNumberText->SetText(FText::FromString(FString::FromInt(CurrentQuantityNumber)));
}

void UShopElementInfoWidget::ChangeRemainingMoneyAfterPurchaseText(int32 MoneyAmount)
{
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

void UShopElementInfoWidget::SetNameInfoTextActive(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		NameInfoText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		NameInfoText->SetVisibility(ESlateVisibility::Hidden);
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

void UShopElementInfoWidget::SetCostInfoActive(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		CostInfoText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		CostInfoText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UShopElementInfoWidget::SetQuantityOverlayActive(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		QuantityOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		QuantityOverlay->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UShopElementInfoWidget::OnBuyButtonClicked()
{
	OnBuyButtonClickedDelegate.Broadcast(CurrentQuantityNumber);
}

void UShopElementInfoWidget::OnIncreaseButtonClicked()
{
	ChangeCurrentQuantityNumber(CurrentQuantityNumber + 1);
}

void UShopElementInfoWidget::OnDecreaseButtonClicked()
{
	ChangeCurrentQuantityNumber(CurrentQuantityNumber - 1);
}

UShopItemMeshPanel* UShopElementInfoWidget::GetItemMeshPanel() const
{
	return ItemMeshPanel;
}
