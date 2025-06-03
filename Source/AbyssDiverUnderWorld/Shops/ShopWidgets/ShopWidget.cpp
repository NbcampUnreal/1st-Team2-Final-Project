#include "ShopWidget.h"

#include "ShopCategoryTabWidget.h"
#include "AbyssDiverUnderWorld.h"
#include "ShopTileView.h"
#include "Shops/ShopItemEntryData.h"
#include "Shops/ShopWidgets/ShopElementInfoWidget.h"
#include "Shops/ShopWidgets/ShopItemMeshPanel.h"

#include "Character/UnderwaterCharacter.h"
#include "Character/UpgradeComponent.h"
#include "DataRow/UpgradeDataRow.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/SoundSubsystem.h"

#include "Components/Button.h"
#include "Components/RichTextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/WidgetAnimation.h"

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
}

void UShopWidget::NativeDestruct()
{
	ConsumableTab->OnShopCategoryTabClickedDelegate.RemoveAll(this);
	EquipmentTab->OnShopCategoryTabClickedDelegate.RemoveAll(this);
	UpgradeTab->OnShopCategoryTabClickedDelegate.RemoveAll(this);
	CloseButton->OnClicked.RemoveAll(this);

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

void UShopWidget::ShowUpgradeInfos(USkeletalMesh* NewUpgradeItemMesh, int32 CurrentUpgradeLevel, bool bIsMaxLevel, int32 CurrentUpgradeCost, const FString& ExtraInfoText)
{
	InfoWidget->ShowUpgradeInfos(NewUpgradeItemMesh, CurrentUpgradeLevel, bIsMaxLevel, CurrentUpgradeCost, ExtraInfoText);
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
