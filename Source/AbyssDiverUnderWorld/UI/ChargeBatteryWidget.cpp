// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ChargeBatteryWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/Button.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "DataRow/FADItemDataRow.h"
#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"

void UChargeBatteryWidget::NativeConstruct()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* DPVRow = DataTableSubsystem->GetItemDataByName("DPV");
		FFADItemDataRow* NVRow = DataTableSubsystem->GetItemDataByName("NightVisionGoggle");
		DPVBatteryMax = DPVRow->Amount;
		NVBatteryMax = NVRow->Amount;
	}
	DPVButton->SetIsEnabled(false);
	DPVBatteryText->SetVisibility(ESlateVisibility::Hidden);
	NVButton->SetIsEnabled(false);
	NVBatteryText->SetVisibility(ESlateVisibility::Hidden);

	GetWorld()->GetTimerManager().SetTimer(InitialzieTimerHandle, this, &UChargeBatteryWidget::InitializeInventory, 2.0f, true);
}

void UChargeBatteryWidget::InitializeInventory()
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

	if (InitializeNum <= 0 || InventoryComp)
	{
		GetWorld()->GetTimerManager().ClearTimer(InitialzieTimerHandle);
		LOGV(Warning, TEXT("InventoryInitialize"));
	}
	else
	{
		LOGV(Warning, TEXT("InventoryInitialize Fail"));
	}
}

void UChargeBatteryWidget::SetEquipBatteryAmount(FName Name, int32 Amount)
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

void UChargeBatteryWidget::SetBatteryButtonActivate(FName Name, bool bActivate)
{
	if (Name == "DPV")
	{
		DPVButton->SetIsEnabled(bActivate);
		DPVBatteryText->SetVisibility(bActivate ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	else if (Name == "NightVisionGoggle")
	{
		NVButton->SetIsEnabled(bActivate);
		NVBatteryText->SetVisibility(bActivate ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

