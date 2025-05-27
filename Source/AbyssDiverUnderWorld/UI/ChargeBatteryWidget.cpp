// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ChargeBatteryWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/Button.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "DataRow/FADItemDataRow.h"
#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Interactable/Item/Component/EquipUseComponent.h"

DEFINE_LOG_CATEGORY(BatteryLog);

void UChargeBatteryWidget::NativeConstruct()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* DPVRow = DataTableSubsystem->GetItemDataByName("DPV");
		FFADItemDataRow* NVRow = DataTableSubsystem->GetItemDataByName("NightVisionGoggle");
		FFADItemDataRow* BatteryRow = DataTableSubsystem->GetItemDataByName("Battery");

		DPVBatteryMax = DPVRow->Amount;
		NVBatteryMax = NVRow->Amount;
		BatteryMax = BatteryRow->Amount;
	}
	DPVButton->SetIsEnabled(false);
	DPVBatteryText->SetVisibility(ESlateVisibility::Hidden);

	NVButton->SetIsEnabled(false);
	NVBatteryText->SetVisibility(ESlateVisibility::Hidden);

	BatteryNumText->SetVisibility(ESlateVisibility::Hidden);
	BatteryAmountText->SetVisibility(ESlateVisibility::Hidden);

	float InitializeRepeatDelay = 2.0f;
	GetWorld()->GetTimerManager().SetTimer(InitialzieTimerHandle, this, &UChargeBatteryWidget::InitializeChargeBatteryWidget, InitializeRepeatDelay, true);
}

void UChargeBatteryWidget::StartChargeBattery(FName ItemName)
{
	CurrentChargeItem = ItemName;
	if (CanCharge())
	{
		float IncreaseRepeatDelay = 0.3f;
		GetWorld()->GetTimerManager().SetTimer(IncreaseTimerHandle, this, &UChargeBatteryWidget::ChargeBatteryAmount, IncreaseRepeatDelay, true);
		LOGB(Warning, TEXT("Start Charge Battery"));
	}
}

void UChargeBatteryWidget::StopChargeBattery()
{
	CurrentChargeItem = NAME_None;

	GetWorld()->GetTimerManager().ClearTimer(IncreaseTimerHandle);
	LOGB(Warning, TEXT("Stop Charge Battery"));
}

bool UChargeBatteryWidget::CanCharge()
{
	if (!InventoryComp) return false;

	if ((CurrentChargeItem == "DPV" && EquipUseComp->bBoostActive) || (CurrentChargeItem == "NightVisionGoggle" && EquipUseComp->bNightVisionOn))
	{
		LOGB(Warning, TEXT("Equipment is in use!"));
		return false;
	}

	const FItemData* BatteryInfo = InventoryComp->GetInventoryItemData("Battery");
	if (BatteryInfo->Quantity > 0)
	{
		LOGB(Warning, TEXT("The battery can be charged"));
		return true;
	}

		
	LOGB(Warning, TEXT("There is no battery."));
	return false;
}

void UChargeBatteryWidget::ChargeBatteryAmount()
{
	if (InventoryComp && EquipUseComp)
	{
		FItemData* CurrentEquipment = InventoryComp->GetCurrentEquipmentItemData();

		int32 MaxToCompare = CurrentChargeItem == "DPV" ? DPVBatteryMax : NVBatteryMax;
		if (CurrentEquipment && CurrentEquipment->Name == CurrentChargeItem)
		{
			if (EquipUseComp->Amount + 1 > MaxToCompare) return;
			EquipUseComp->S_IncreaseAmount(1);
			LOGB(Warning, TEXT("Amount of EquipUseComp is charged"));
		}
		else
		{
			const FItemData* ItemInfoToCharge = InventoryComp->GetInventoryItemData(CurrentChargeItem);
			if (CurrentChargeItem == NAME_None) return;
			if (ItemInfoToCharge->Amount + 1 > MaxToCompare) return;

			InventoryComp->S_EquipmentChargeBattery(CurrentChargeItem, 1); 
			LOGB(Warning, TEXT("Amount of InventoryComp is charged"));
		}
		InventoryComp->S_UseBatteryAmount(-1);
		UpdateBatteryInfo();
	}
}

void UChargeBatteryWidget::UpdateBatteryInfoDelay()
{
	const FItemData* BatteryData = InventoryComp->GetInventoryItemData("Battery");
	int8 BatteryQuantity = BatteryData->Quantity;
	int8 BatteryAmount = BatteryData->Amount;
	if (BatteryQuantity <= 0)
	{
		BatteryNumText->SetVisibility(ESlateVisibility::Hidden);
		BatteryAmountText->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		BatteryNumText->SetVisibility(ESlateVisibility::Visible);
		BatteryAmountText->SetVisibility(ESlateVisibility::Visible);
	}

	int8 Percent = FMath::RoundToInt((float)BatteryAmount / BatteryMax * 100.0f);
	BatteryNumText->SetText(FText::FromString(FString::Printf(TEXT("%d"), BatteryQuantity)));
	BatteryAmountText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
}

void UChargeBatteryWidget::UpdateBatteryInfo()
{
	FTimerHandle UpdateBatteryInfoTimerHandle;

	float UpdateDelay = 1.0f;
	GetWorld()->GetTimerManager().SetTimer(UpdateBatteryInfoTimerHandle, this, &UChargeBatteryWidget::UpdateBatteryInfoDelay, UpdateDelay, false);
}

void UChargeBatteryWidget::InitializeChargeBatteryWidget()
{
	InitializeNum--;
	if (InventoryComp == nullptr)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AADPlayerState* PS = Cast<AADPlayerState>(PC->PlayerState);
			InventoryComp = PS->GetInventory();
			InventoryComp->SetChargeBatteryInstance(this);
		}
	}
	if (EquipUseComp == nullptr)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			if(AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(PC->GetPawn()))
				EquipUseComp = Character->GetEquipUseComponent();
		}
	}
	

	if (InitializeNum <= 0 || (InventoryComp && EquipUseComp))
	{
		GetWorld()->GetTimerManager().ClearTimer(InitialzieTimerHandle);
		LOGB(Warning, TEXT("ChargeBattery Initialize"));
	}
	else
	{
		LOGB(Warning, TEXT("ChargeBattery Initialize Fail"));
	}
}

void UChargeBatteryWidget::SetEquipBatteryAmount(FName Name, int16 Amount)
{
	if (Name == "DPV")
	{
		int8 Percent = FMath::RoundToInt((float)Amount / DPVBatteryMax * 100.0f);
		DPVBatteryText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
	}
	else if (Name == "NightVisionGoggle")
	{
		int8 Percent = FMath::RoundToInt((float)Amount / NVBatteryMax * 100.0f);
		NVBatteryText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
	}
}

void UChargeBatteryWidget::SetEquipBatteryButtonActivate(FName Name, bool bActivate)
{
	if (Name == "DPV")
	{
		DPVButton->SetIsEnabled(bActivate);
		DPVBatteryText->SetVisibility(bActivate ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		if (bActivate)
		{
			LOGB(Warning, TEXT("Activate DPV"));
		}
		else
		{
			LOGB(Warning, TEXT("DeActivate DPV"));
		}
	}
	else if (Name == "NightVisionGoggle")
	{
		NVButton->SetIsEnabled(bActivate);
		NVBatteryText->SetVisibility(bActivate ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (bActivate)
	{
		LOGB(Warning, TEXT("Activate NV"));
	}
	else
	{
		LOGB(Warning, TEXT("DeActivate NV"));
	}
}



