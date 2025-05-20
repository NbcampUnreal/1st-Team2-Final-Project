#include "ShopElementInfoWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "Shops/ShopWidgets/ShopItemMeshPanel.h"
#include "Framework/ADInGameState.h"

#include "Components/RichTextBlock.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void UShopElementInfoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

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


	
	bIsStackableItem = false;
	bIsShowingUpgradeView = false;
}

void UShopElementInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (ensureMsgf(GS, TEXT("GS 캐스팅 실패, 게임 모드 확인 부탁.")) == false)
	{
		return;
	}

	GS->TeamCreditsChangedDelegate.AddUObject(this, &UShopElementInfoWidget::OnTeamCreditChanged);
}

void UShopElementInfoWidget::NativeDestruct()
{
	AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (ensureMsgf(GS, TEXT("GS 캐스팅 실패, 게임 모드 확인 부탁.")) == false)
	{
		return;
	}

	GS->TeamCreditsChangedDelegate.RemoveAll(this);

	Super::NativeDestruct();
}

void UShopElementInfoWidget::Init(USkeletalMeshComponent* NewItemMeshComp)
{
	ItemMeshPanel->Init(NewItemMeshComp);
	ChangeCurrentQuantityNumber(0);

	SetItemMeshActive(false);
	SetDescriptionActive(false);
	SetNameInfoTextActive(false);
	SetBuyButtonActive(false);
	SetUpgradeLevelInfoActive(false);
	SetCostInfoActive(false);
	SetQuantityOverlayActive(false);
	SetRemainingMoneyAfterPurchaseTextActive(false);
}

void UShopElementInfoWidget::ShowItemInfos(USkeletalMesh* NewItemMesh, const FString& NewDescription, const FString& NewNameInfoText, int32 ItemCost, bool bIsStackable)
{
	CurrentCost = ItemCost;
	bIsStackableItem = bIsStackable;
	ChangeCurrentQuantityNumber(1);

	SetItemMeshActive(true);
	SetDescriptionActive(true);
	SetNameInfoTextActive(true);
	SetBuyButtonActive(true);
	SetUpgradeLevelInfoActive(false);
	SetCostInfoActive(true);
	SetQuantityOverlayActive(true); 
	SetRemainingMoneyAfterPurchaseTextActive(true);

	ChangeItemMesh(NewItemMesh);
	ChangeItemDescription(NewDescription);
	ChangeNameInfoText(NewNameInfoText);
	
	ChangeCostInfo(ItemCost, false);
	ChangeRemainingMoneyAfterPurchaseTextFromCost(ItemCost);

	bIsShowingUpgradeView = false;
}

void UShopElementInfoWidget::ShowUpgradeInfos(USkeletalMesh* NewUpgradeItemMesh, int32 CurrentUpgradeLevel, bool bIsMaxLevel, int32 CurrentUpgradeCost, const FString& ExtraInfoText)
{
	CurrentCost = CurrentUpgradeCost;
	bIsShowingUpgradeView = true;
	ChangeCurrentQuantityNumber(1);

	SetItemMeshActive(true);
	SetDescriptionActive(false);
	SetNameInfoTextActive(false);
	SetBuyButtonActive(true);
	SetUpgradeLevelInfoActive(true);
	SetCostInfoActive(true);
	SetQuantityOverlayActive(false);
	SetRemainingMoneyAfterPurchaseTextActive(true);

	ChangeItemMesh(NewUpgradeItemMesh);
	ChangeUpgradeLevelInfo(CurrentUpgradeLevel, bIsMaxLevel);
	ChangeCostInfo(CurrentUpgradeCost, true);

	
	ChangeRemainingMoneyAfterPurchaseTextFromCost(CurrentUpgradeCost);


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

void UShopElementInfoWidget::ChangeCostInfo(int32 Cost, bool bIsUpgradeCost)
{
	FString NewInfoText = "";

	if (bIsUpgradeCost)
	{
		NewInfoText = TEXT("<S>업그레이드 비용 ");
		NewInfoText += FString::FromInt(Cost);
		NewInfoText += TEXT("Cr</>");
	}
	else
	{
		NewInfoText = TEXT("<S>비용 ");
		NewInfoText += FString::Printf(TEXT("%d * %d Cr </>"), Cost, CurrentQuantityNumber);
	}

	CostInfoText->SetText(FText::FromString(NewInfoText));
}

void UShopElementInfoWidget::ChangeCurrentQuantityNumber(int32 NewNumber)
{
	AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GS == nullptr)
	{
		LOGV(Warning, TEXT("GS == nullptr"));
		return;
	}

	if (bIsStackableItem)
	{
		int32 MaxCount = (CurrentCost == 0) ? MAX_ITEM_COUNT : FMath::Min(MAX_ITEM_COUNT, GS->GetTotalTeamCredit() / CurrentCost);
		if (MaxCount == 0)
		{
			NewNumber = 0;
		}
		else //if (MaxCount < NewNumber)
		{
			NewNumber = FMath::Clamp(NewNumber, 1, MaxCount);
		}
	}
	else
	{
		NewNumber = (GS->GetTotalTeamCredit() >= CurrentCost) ? 1 : 0;
	}

	CurrentQuantityNumber = NewNumber;
	CurrentQuantityNumberText->SetText(FText::FromString(FString::FromInt(CurrentQuantityNumber)));
}

void UShopElementInfoWidget::ChangeRemainingMoneyAfterPurchaseTextFromCost(int32 Cost)
{
	AADInGameState* GS = CastChecked<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	int32 RemainingMoney = GS->GetTotalTeamCredit() - Cost * CurrentQuantityNumber;
	ChangeRemainingMoneyAfterPurchaseText(RemainingMoney);
}

void UShopElementInfoWidget::ChangeRemainingMoneyAfterPurchaseText(int32 MoneyAmount)
{
	FString NewText = TEXT("<S>구매 후 남는 잔액 ");
	NewText += FString::FromInt(MoneyAmount);
	NewText += TEXT("Cr</>");

	RemainingMoneyAfterPurchaseText->SetText(FText::FromString(NewText));
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

void UShopElementInfoWidget::SetRemainingMoneyAfterPurchaseTextActive(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		RemainingMoneyAfterPurchaseText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		RemainingMoneyAfterPurchaseText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UShopElementInfoWidget::OnBuyButtonClicked()
{
	OnBuyButtonClickedDelegate.Broadcast(CurrentQuantityNumber);
}

void UShopElementInfoWidget::OnIncreaseButtonClicked()
{
	ChangeCurrentQuantityNumber(CurrentQuantityNumber + 1);
	ChangeRemainingMoneyAfterPurchaseTextFromCost(CurrentCost);
	ChangeCostInfo(CurrentCost, bIsShowingUpgradeView);
}

void UShopElementInfoWidget::OnDecreaseButtonClicked()
{
	ChangeCurrentQuantityNumber(CurrentQuantityNumber - 1);
	ChangeRemainingMoneyAfterPurchaseTextFromCost(CurrentCost);
	ChangeCostInfo(CurrentCost, bIsShowingUpgradeView);
}

void UShopElementInfoWidget::OnTeamCreditChanged(int32 ChangedValue)
{
	ChangeRemainingMoneyAfterPurchaseTextFromCost(CurrentCost);
}

UShopItemMeshPanel* UShopElementInfoWidget::GetItemMeshPanel() const
{
	return ItemMeshPanel;
}