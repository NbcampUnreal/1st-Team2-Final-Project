// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ADNightVisionGoggle.h"
#include "ADNightVisionGoggle.h"
#include "Components/ProgressBar.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/SoundSubsystem.h"
#include "DataRow/FADItemDataRow.h"
#include "DataRow/SoundDataRow/SFXDataRow.h"

void UADNightVisionGoggle::NativeConstruct()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* Row = DataTableSubsystem->GetItemDataByName("NightVisionGoggle");
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
		BatteryMax = Row->Amount;
	}
}

void UADNightVisionGoggle::NightVigionUnUse_Implementation()
{
	GetSoundSubsystem()->Play2D(ESFX::NVOff);
}

void UADNightVisionGoggle::NightVigionUse_Implementation()
{
	GetSoundSubsystem()->Play2D(ESFX::NVOn);
}

void UADNightVisionGoggle::SetBatteryAmount(int32 Amount)
{
	if (BatteryAmount && BatteryMax > 0)
	{
		BatteryAmount->SetPercent(static_cast<float>(Amount) / BatteryMax);
	}
}

USoundSubsystem* UADNightVisionGoggle::GetSoundSubsystem()
{
	if (SoundSubsystem)
	{
		return SoundSubsystem;
	}

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
		return SoundSubsystem;
	}
	return nullptr;
}