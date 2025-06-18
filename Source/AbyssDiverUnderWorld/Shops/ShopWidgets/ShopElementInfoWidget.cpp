#include "ShopElementInfoWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "Shops/ShopWidgets/ShopItemMeshPanel.h"
#include "Framework/ADInGameState.h"
#include "DataRow/FADItemDataRow.h"
#include "DataRow/UpgradeDataRow.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/SoundSubsystem.h"

#include "Components/RichTextBlock.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void UShopElementInfoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

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

	if (AddButton->OnClicked.IsBound() == false)
	{
		AddButton->OnClicked.AddDynamic(this, &UShopElementInfoWidget::OnAddButtonClicked);
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

void UShopElementInfoWidget::NativeDestruct()
{
	AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (ensureMsgf(GS, TEXT("GS 캐스팅 실패, 게임 모드 확인 부탁.")) == false)
	{
		return;
	}

	GS->TeamCreditsChangedDelegate.RemoveAll(this);
	AddButton->OnClicked.RemoveAll(this);
	IncreaseButton->OnClicked.RemoveAll(this);
	DecreaseButton->OnClicked.RemoveAll(this);

	Super::NativeDestruct();
}

void UShopElementInfoWidget::Init(USkeletalMeshComponent* NewItemMeshComp)
{
	ItemMeshPanel->Init(NewItemMeshComp);
	ChangeCurrentQuantityNumber(0);

	SetItemMeshActive(false);
	SetDescriptionActive(false);
	SetNameInfoTextActive(false);
	SetAddButtonActive(false);
	SetUpgradeLevelInfoActive(false);
	SetCostInfoActive(false);
	SetQuantityOverlayActive(false);

	ItemMeshPanel->SetUpgradeImageActive(false);
}

void UShopElementInfoWidget::ShowItemInfos(int32 ItemId)
{
	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Warning, TEXT("DataTableSubsystem == nullptr"));
		return;
	}

	FFADItemDataRow* ItemData = DataTableSubsystem->GetItemData(ItemId);
	if (ItemData == nullptr)
	{
		LOGV(Warning, TEXT("ItemData == nullptr"));
		return;
	}

	CurrentCost = ItemData->Price;
	bIsStackableItem = ItemData->Stackable;
	ChangeCurrentQuantityNumber(1);

	SetItemMeshActive(true);
	SetDescriptionActive(true);
	SetNameInfoTextActive(true);
	SetAddButtonActive(true);
	SetUpgradeLevelInfoActive(false);
	SetCostInfoActive(true);
	SetQuantityOverlayActive(true); 

	ChangeItemMesh(ItemData->SkeletalMesh, ItemId);
	ChangeDescription(ItemData->Description);
	ChangeNameInfoText(ItemData->Name.ToString());
	
	ChangeCostInfo(ItemData->Price, false);

	bIsShowingUpgradeView = false;

	ItemMeshPanel->SetUpgradeImageActive(false);
}

void UShopElementInfoWidget::ShowUpgradeInfos(EUpgradeType UpgradeType, uint8 Grade, bool bIsMaxLevel)
{
	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Warning, TEXT("DataTableSubsystem == nullptr"));
		return;
	}

	FUpgradeDataRow* UpgradeData = nullptr;
	if (bIsMaxLevel)
	{
		UpgradeData = DataTableSubsystem->GetUpgradeData(UpgradeType, Grade);
		if (UpgradeData == nullptr)
		{
			LOGV(Error, TEXT("UpgradeData == nullptr"));
			return;
		}

		CurrentCost = 0;
	}
	else
	{
		UpgradeData = DataTableSubsystem->GetUpgradeData(UpgradeType, Grade + 1);
		if (UpgradeData == nullptr)
		{
			LOGV(Error, TEXT("UpgradeData == nullptr"));
			return;
		}

		CurrentCost = UpgradeData->Price;
	}

	ItemMeshPanel->ChangeUpgradeImage(UpgradeData->UpgradeIcon);
	ItemMeshPanel->SetUpgradeImageActive(true);

	bIsShowingUpgradeView = true;
	ChangeCurrentQuantityNumber(1);

	SetItemMeshActive(false);
	SetDescriptionActive(true);
	SetNameInfoTextActive(true);
	SetAddButtonActive(false);
	SetUpgradeLevelInfoActive(true);
	SetCostInfoActive(true);
	SetQuantityOverlayActive(false);
	
	ChangeItemMesh(nullptr, INDEX_NONE);
	ChangeDescription(UpgradeData->UpgradeDescription);
	ChangeNameInfoText(UpgradeData->UpgradeName);
	ChangeUpgradeLevelInfo(Grade, bIsMaxLevel);
	ChangeCostInfo(CurrentCost, true);
}

void UShopElementInfoWidget::ChangeDescription(const FString& NewDescription)
{
	DescriptionText->SetText(FText::FromString(NewDescription));
}

void UShopElementInfoWidget::ChangeNameInfoText(const FString& NewInfoText)
{
	NameInfoText->SetText(FText::FromString(NewInfoText));
}

void UShopElementInfoWidget::ChangeItemMesh(USkeletalMesh* NewMesh, int32 ItemId)
{
	ItemMeshPanel->ChangeItemMesh(NewMesh, ItemId);
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

	int32 MaxCount = (CurrentCost == 0) ? MAX_ITEM_COUNT : FMath::Min(MAX_ITEM_COUNT, GS->GetTotalTeamCredit() / CurrentCost);
	if (MaxCount == 0)
	{
		NewNumber = 0;
	}
	else //if (MaxCount < NewNumber)
	{
		NewNumber = FMath::Clamp(NewNumber, 1, MaxCount);
	}

	/*if (bIsStackableItem)
	{
		
	}
	else
	{
		NewNumber = (GS->GetTotalTeamCredit() >= CurrentCost) ? 1 : 0;
	}*/

	CurrentQuantityNumber = NewNumber;
	CurrentQuantityNumberText->SetText(FText::FromString(FString::FromInt(CurrentQuantityNumber)));
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

void UShopElementInfoWidget::SetAddButtonActive(bool bShouldActivate)
{
	if (bShouldActivate)
	{
		AddButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		AddButton->SetVisibility(ESlateVisibility::Hidden);
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

void UShopElementInfoWidget::OnAddButtonClicked()
{
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Play2D(ESFX_UI::UIClicked);
	OnAddButtonClickedDelegate.Broadcast(CurrentQuantityNumber);
}

void UShopElementInfoWidget::OnIncreaseButtonClicked()
{
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Play2D(ESFX_UI::UIClicked);
	ChangeCurrentQuantityNumber(CurrentQuantityNumber + 1);
	ChangeCostInfo(CurrentCost, bIsShowingUpgradeView);
}

void UShopElementInfoWidget::OnDecreaseButtonClicked()
{
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Play2D(ESFX_UI::UIClicked);
	ChangeCurrentQuantityNumber(CurrentQuantityNumber - 1);
	ChangeCostInfo(CurrentCost, bIsShowingUpgradeView);
}

void UShopElementInfoWidget::OnTeamCreditChanged(int32 ChangedValue)
{
	// 돈소리?
}

UShopItemMeshPanel* UShopElementInfoWidget::GetItemMeshPanel() const
{
	return ItemMeshPanel;
}