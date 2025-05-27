// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ADNightVisionGoggle.h"
#include "ADNightVisionGoggle.h"
#include "Components/ProgressBar.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "DataRow/FADItemDataRow.h"

void UADNightVisionGoggle::NativeConstruct()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* Row = DataTableSubsystem->GetItemDataByName("NightVisionGoggle");
		BatteryMax = Row->Amount;
	}
}

void UADNightVisionGoggle::NightVigionUnUse_Implementation()
{
}

void UADNightVisionGoggle::NightVigionUse_Implementation()
{

}

void UADNightVisionGoggle::SetBatteryAmount(int32 Amount)
{
	if (BatteryAmount && BatteryMax > 0)
	{
		BatteryAmount->SetPercent(static_cast<float>(Amount) / BatteryMax);
	}
}
