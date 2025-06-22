#include "ShopWidget.h"

#include "ShopCategoryTabWidget.h"
#include "AbyssDiverUnderWorld.h"
#include "ShopTileView.h"
#include "Shops/ShopItemEntryData.h"
#include "Shops/ShopBuyListEntryData.h"
#include "Shops/ShopWidgets/ShopElementInfoWidget.h"
#include "Shops/ShopWidgets/ShopItemMeshPanel.h"
#include "Shops/ShopWidgets/ShopBuyListSlotWidget.h"

#include "Character/UnderwaterCharacter.h"
#include "Character/UpgradeComponent.h"
#include "DataRow/UpgradeDataRow.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/SoundSubsystem.h"
#include "DataRow/FADItemDataRow.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/WidgetAnimation.h"

const int32 UShopWidget::MaxItemCount = 99;

void UShopWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CurrentActivatedTab = EShopCategoryTab::Equipment;
}

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ConsumableTab->OnShopCategoryTabClickedDelegate.AddUObject(this, &UShopWidget::OnCategoryTabClicked);
	EquipmentTab->OnShopCategoryTabClickedDelegate.AddUObject(this, &UShopWidget::OnCategoryTabClicked);
	UpgradeTab->OnShopCategoryTabClickedDelegate.AddUObject(this, &UShopWidget::OnCategoryTabClicked);
	CloseButton->OnClicked.AddDynamic(this, &UShopWidget::OnCloseButtonClicked);
	BuyButton->OnClicked.AddDynamic(this, &UShopWidget::OnBuyButtonClicked);
}

void UShopWidget::NativeDestruct()
{
	ConsumableTab->OnShopCategoryTabClickedDelegate.RemoveAll(this);
	EquipmentTab->OnShopCategoryTabClickedDelegate.RemoveAll(this);
	UpgradeTab->OnShopCategoryTabClickedDelegate.RemoveAll(this);
	CloseButton->OnClicked.RemoveAll(this);
	BuyButton->OnClicked.RemoveAll(this);

	Super::NativeDestruct();
}

FReply UShopWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UShopItemMeshPanel* MeshPanel = InfoWidget->GetItemMeshPanel();
	MeshPanel->SetMouseDown(false);

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UShopWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UShopItemMeshPanel* MeshPanel = InfoWidget->GetItemMeshPanel();

	if (MeshPanel->GetMouseDown())
	{
		float CurrentMouseX = InMouseEvent.GetScreenSpacePosition().X;
		float MouseX = MeshPanel->GetCurrentMousePositionY();
		float DeltaX = (MouseX - CurrentMouseX) * MESH_ROTATION_SPEED;

		MeshPanel->SetCurrentMousePositionX(CurrentMouseX);
		MeshPanel->AddMeshRotationYaw(DeltaX);
	}
	
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UShopWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	/*if (InKeyEvent.GetKey() == EKeys::Escape)
	{
	나중에 사용할수도?
	}*/

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UShopWidget::SetAllItems(const TArray<UShopItemEntryData*>& EntryDataList, EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	switch (Tab)
	{
	case EShopCategoryTab::Consumable:
		ConsumableTabEntryDataList = EntryDataList;
		break;
	case EShopCategoryTab::Equipment:
		EquipmentTabEntryDataList = EntryDataList;
		break;
	case EShopCategoryTab::Upgrade:
		UpgradeTabEntryDataList = EntryDataList;
		return;
	case EShopCategoryTab::Max:
		check(false);
		return;
	default:
		check(false);
		return;
	}
}

void UShopWidget::AddItem(UShopItemEntryData* EntryData, EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	switch (Tab)
	{
	case EShopCategoryTab::Consumable:
		ConsumableTabEntryDataList.Add(EntryData);
		break;
	case EShopCategoryTab::Equipment:
		EquipmentTabEntryDataList.Add(EntryData);
		break;
	case EShopCategoryTab::Upgrade:
		UpgradeTabEntryDataList.Add(EntryData);
		return;
	case EShopCategoryTab::Max:
		check(false);
		return;
	default:
		check(false);
		return;
	}
}

void UShopWidget::RemoveItem(int32 Index, EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	switch (Tab)
	{
	case EShopCategoryTab::Consumable:
		ConsumableTabEntryDataList.RemoveAt(Index);
		break;
	case EShopCategoryTab::Equipment:
		EquipmentTabEntryDataList.RemoveAt(Index);
		break;
	case EShopCategoryTab::Upgrade:
		UpgradeTabEntryDataList.RemoveAt(Index);
		return;
	case EShopCategoryTab::Max:
		check(false);
		return;
	default:
		check(false);
		return;
	}
}

void UShopWidget::ModifyItem(int32 Index, UTexture2D* NewItemImage, const FString& NewToolTipText, EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	switch (Tab)
	{
	case EShopCategoryTab::Consumable:
		ConsumableTabEntryDataList[Index]->Init(Index, NewItemImage, NewToolTipText);
		break;
	case EShopCategoryTab::Equipment:
		EquipmentTabEntryDataList[Index]->Init(Index, NewItemImage, NewToolTipText);
		break;
	case EShopCategoryTab::Upgrade:
		UpgradeTabEntryDataList[Index]->Init(Index, NewItemImage, NewToolTipText);
		return;
	case EShopCategoryTab::Max:
		check(false);
		return;
	default:
		check(false);
		return;
	}
}

void UShopWidget::ShowItemViewForTab(EShopCategoryTab TabType)
{
	if (TabType >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), TabType);
		return;
	}

	if (TabType != CurrentActivatedTab)
	{
		SetCurrentActivatedTab(TabType);
	}

	RefreshItemView();
}

void UShopWidget::RefreshItemView()
{
	check(ItemTileView);

	switch (CurrentActivatedTab)
	{
	case EShopCategoryTab::Consumable:
		ItemTileView->SetAllElements(ConsumableTabEntryDataList);
		ItemTileView->SetVisibility(ESlateVisibility::Visible);
		UpgradeTileView->SetVisibility(ESlateVisibility::Hidden);

		break;
	case EShopCategoryTab::Equipment:
		ItemTileView->SetAllElements(EquipmentTabEntryDataList);
		ItemTileView->SetVisibility(ESlateVisibility::Visible);
		UpgradeTileView->SetVisibility(ESlateVisibility::Hidden);

		break;
	case EShopCategoryTab::Upgrade:
		UpgradeTileView->SetAllElements(UpgradeTabEntryDataList);
		UpgradeTileView->SetVisibility(ESlateVisibility::Visible);
		ItemTileView->SetVisibility(ESlateVisibility::Hidden);

		return;
	case EShopCategoryTab::Max:
		check(false);
		return;
	default:
		check(false);
		return;
	}
}

void UShopWidget::ShowItemInfos(int32 ItemId)
{
	InfoWidget->ShowItemInfos(ItemId);
}

void UShopWidget::ShowUpgradeInfos(EUpgradeType UpgradeType, uint8 Grade, bool bIsMaxLevel)
{
	InfoWidget->ShowUpgradeInfos(UpgradeType, Grade, bIsMaxLevel);
}

void UShopWidget::SetTeamMoneyText(int32 NewTeamMoney)
{
	FString NewTeamMoneyText = TEXT("공통 자금 : ");
	NewTeamMoneyText += FString::FromInt(NewTeamMoney);
	NewTeamMoneyText += TEXT(" Cr");

	TeamMoneyText->SetText(FText::FromString(NewTeamMoneyText));
}

void UShopWidget::PlayCloseAnimation()
{
	PlayAnimation(CloseShopAnim);
}

UShopBuyListEntryData* UShopWidget::AddToBuyList(uint8 ItemId, int32& InOutIncreasedAmount)
{
	if (InOutIncreasedAmount < 1 || InOutIncreasedAmount > MaxItemCount)
	{
		return nullptr;
	}

	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubsystem == nullptr"));
		return nullptr;
	}

	FFADItemDataRow* ItemData = DataTableSubsystem->GetItemData(ItemId);
	if (ItemData == nullptr)
	{
		LOGV(Error, TEXT("ItemData == nullptr"));
		return nullptr;
	}

	UShopBuyListEntryData* BuyListEntryData = nullptr;

	int32 Index = Contains(ItemId);
	if (Index == INDEX_NONE)
	{
		BuyListEntryData = NewObject<UShopBuyListEntryData>(this);
		BuyListEntryData->Init(BuyListEntryDataList.Num(), ItemData->Thumbnail, TEXT(""));
		BuyListEntryData->SetItemCount(InOutIncreasedAmount);
		BuyListEntryDataList.Emplace(BuyListEntryData);
		BuyListEntryItemIdList.Emplace(ItemId);
		BuyListTileView->AddItem(BuyListEntryData);
	}
	else
	{
		BuyListEntryData = BuyListEntryDataList[Index];
		if (BuyListEntryData == nullptr)
		{
			LOGV(Error, TEXT("BuyListEntryData == nullptr"));
			return nullptr;
		}
		const int32 CurrentItemCount = BuyListEntryData->GetItemCount();
		int32 ActualAmount = FMath::Clamp(CurrentItemCount + InOutIncreasedAmount, 0, MaxItemCount);
		BuyListEntryData->SetItemCount(ActualAmount);
		
		UShopBuyListSlotWidget* BuyListSlotWidget = BuyListTileView->GetEntryWidgetFromItem<UShopBuyListSlotWidget>(BuyListEntryData);
		if (BuyListSlotWidget == nullptr)
		{
			LOGV(Error, TEXT("Fail to get BuyListSlotWidget From BuyListEntryData"));
			return nullptr;
		}

		BuyListSlotWidget->SetItemCountText(BuyListEntryData->GetItemCount());
		InOutIncreasedAmount = ActualAmount - CurrentItemCount;
	}

	return BuyListEntryData;
}

void UShopWidget::RemoveBuyListAt(int32 ListIndex, int32 Amount)
{
	if (BuyListEntryDataList.IsValidIndex(ListIndex) == false)
	{
		LOGV(Error, TEXT("Not Valid Index : %d"), ListIndex);
		return;
	}

	if (BuyListEntryItemIdList.IsValidIndex(ListIndex) == false)
	{
		LOGV(Error, TEXT("Not Valid Index : %d"), ListIndex);
		return;
	}

	UShopBuyListEntryData* BuyListEntryData = BuyListEntryDataList[ListIndex];
	check(BuyListEntryData);

	if (BuyListEntryData->GetItemCount() <= Amount)
	{
		BuyListEntryDataList.RemoveAt(ListIndex);
		BuyListEntryItemIdList.RemoveAt(ListIndex);
		BuyListTileView->RemoveItem(BuyListEntryData);

		const int32 EntryCount = BuyListEntryDataList.Num();
		for (int32 i = ListIndex; i < EntryCount; ++i)
		{
			UShopBuyListEntryData* NextSlotData = BuyListEntryDataList[i];
			if (IsValid(NextSlotData) == false)
			{
				LOGV(Error, TEXT("Slot Data Is Invalid, Index : %d"), i);
				return;
			}

			NextSlotData->SetSlotIndex(i);

			UShopBuyListSlotWidget* NextSlotWidget = BuyListTileView->GetEntryWidgetFromItem<UShopBuyListSlotWidget>(NextSlotData);
			if (NextSlotWidget == nullptr)
			{
				LOGV(Error, TEXT("Fail to get NextSlotWidget From BuyListEntryData"));
				return;
			}

			NextSlotWidget->SetSlotIndex(i);
		}
	}
	else
	{
		UShopBuyListSlotWidget* BuyListSlotWidget = BuyListTileView->GetEntryWidgetFromItem<UShopBuyListSlotWidget>(BuyListEntryData);
		if (BuyListSlotWidget == nullptr)
		{
			LOGV(Error, TEXT("Fail to get BuyListSlotWidget From BuyListEntryData"));
			return;
		}

		int32 RemainingCount = BuyListEntryData->GetItemCount() - Amount;
		BuyListSlotWidget->SetItemCountText(RemainingCount);
		BuyListEntryData->SetItemCount(RemainingCount);
	}
}

void UShopWidget::RemoveBuyListAll()
{
	BuyListEntryDataList.Reset();
	BuyListEntryItemIdList.Reset();
	BuyListTileView->SetAllElements((TArray<UShopItemEntryData*>)BuyListEntryDataList);
}

int32 UShopWidget::Contains(uint8 ItemId)
{
	const int32 ListCount = BuyListEntryItemIdList.Num();
	for (int32 i = 0; i < ListCount; ++i)
	{
		if (BuyListEntryItemIdList[i] == ItemId)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

void UShopWidget::ChangeTotalPriceText(int32 NewTotalPrice)
{
	FString NewText = TEXT("전체 가격 : ") + FString::FromInt(NewTotalPrice) + TEXT(" Cr");
	TotalPriceText->SetText(FText::FromString(NewText));
}

void UShopWidget::OnCategoryTabClicked(EShopCategoryTab CategoryTab)
{
	if (CategoryTab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), CategoryTab);
		return;
	}

	ShowItemViewForTab(CategoryTab);
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Play2D(ESFX_UI::UIClicked);
}

void UShopWidget::OnCloseButtonClicked()
{
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Play2D(ESFX_UI::UIClicked);
	OnShopCloseButtonClickedDelegate.Broadcast();
}

void UShopWidget::OnBuyButtonClicked()
{
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Play2D(ESFX_UI::UIClicked);
	OnShopBuyButtonClickedDelegate.Broadcast();
}

EShopCategoryTab UShopWidget::GetCurrentActivatedTab() const
{
	return CurrentActivatedTab;
}

void UShopWidget::SetCurrentActivatedTab(EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	CurrentActivatedTab = Tab;
}

UShopCategoryTabWidget* UShopWidget::GetCategoryTab(EShopCategoryTab CategoryTab) const
{
	if (CategoryTab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), CategoryTab);
		return nullptr;
	}

	UShopCategoryTabWidget* Tab = nullptr;

	switch (CategoryTab)
	{
	case EShopCategoryTab::Consumable:
		Tab = ConsumableTab;
		break;
	case EShopCategoryTab::Equipment:
		Tab = EquipmentTab;
		break;
	case EShopCategoryTab::Upgrade:
		LOGV(Error, TEXT("Upgrade Tab is not Supported Currently"));
		return nullptr;
	case EShopCategoryTab::Max:
		check(false);
		return nullptr;
	default:
		check(false);
		return nullptr;
	}

	return Tab;
}

UShopElementInfoWidget* UShopWidget::GetInfoWidget() const
{
	check(InfoWidget);
	return InfoWidget;
}

TArray<TObjectPtr<UShopItemEntryData>>& UShopWidget::GetUpgradeTabEntryDataList()
{
	return UpgradeTabEntryDataList;
}

float UShopWidget::GetCloseShopAnimEndTime() const
{
	return CloseShopAnim->GetEndTime();
}
