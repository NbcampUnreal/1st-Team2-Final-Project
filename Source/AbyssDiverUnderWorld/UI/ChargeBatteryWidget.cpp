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
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Animation/WidgetAnimation.h"


DEFINE_LOG_CATEGORY(BatteryLog);

void UChargeBatteryWidget::NativeConstruct()
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		DPVRow = DataTableSubsystem->GetItemData(static_cast<int8>(EChargeBatteryType::DPV));
		NVRow = DataTableSubsystem->GetItemData(static_cast<int8>(EChargeBatteryType::NightVisionGoggle));
		BatteryRow = DataTableSubsystem->GetItemData(static_cast<int8>(EChargeBatteryType::Battery));

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

	if (SoundCue)
	{
		UAudioComponent* Sound = UGameplayStatics::SpawnSound2D(this, SoundCue, 1.0f, 1.0f, 0.0f, nullptr, true);
		if (Sound)
		{
			Sound->Play();
			ChargeBatterySound = Sound;
		}
	}

	if (CanCharge())
	{
		float IncreaseRepeatDelay = 0.2f;
		GetWorld()->GetTimerManager().SetTimer(IncreaseTimerHandle, this, &UChargeBatteryWidget::ChargeBatteryAmount, IncreaseRepeatDelay, true);
		LOGB(Warning, TEXT("Start Charge Battery"));
	}
}

void UChargeBatteryWidget::StopChargeBattery()
{
	CurrentChargeItem = NAME_None;

	if (ChargeBatterySound && ChargeBatterySound->IsPlaying())
	{
		ChargeBatterySound->Stop();
	}

	GetWorld()->GetTimerManager().ClearTimer(IncreaseTimerHandle);
	LOGB(Warning, TEXT("Stop Charge Battery"));
}

bool UChargeBatteryWidget::CanCharge()
{
	if (!InventoryComp) return false;

	if ((CurrentChargeItem == DPVRow->Name && EquipUseComp->bBoostActive) || (CurrentChargeItem == NVRow->Name && EquipUseComp->bNightVisionOn))
	{
		LOGB(Warning, TEXT("Equipment is in use!"));
		return false;
	}

	const FItemData* BatteryInfo = InventoryComp->GetInventoryItemData(BatteryRow->Name);
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
	if (!CanCharge()) return;
	if (InventoryComp && EquipUseComp)
	{
		FItemData* CurrentEquipment = InventoryComp->GetCurrentEquipmentItemData();

		int32 MaxToCompare = CurrentChargeItem == DPVRow->Name ? DPVBatteryMax : NVBatteryMax;
		if (CurrentChargeItem == NAME_None) return;
		float ChargeRate = 0.01f;
		int32 IncreaseAmount = FMath::Max(1, FMath::RoundToInt(MaxToCompare * ChargeRate));

		if (CurrentEquipment && CurrentEquipment->Name == CurrentChargeItem)
		{
			if (EquipUseComp->Amount >= MaxToCompare)
			{
				StopChargeBattery();
				return;
			}
			EquipUseComp->S_IncreaseAmount(IncreaseAmount);
			LOGB(Warning, TEXT("Amount of EquipUseComp is charged"));
		}
		else
		{
			const FItemData* ItemInfoToCharge = InventoryComp->GetInventoryItemData(CurrentChargeItem);
			if (ItemInfoToCharge->Amount >= MaxToCompare)
			{
				StopChargeBattery();
				return;
			}
			InventoryComp->S_EquipmentChargeBattery(ChargeBatteryTypeMap[CurrentChargeItem], IncreaseAmount);
			LOGB(Warning, TEXT("Amount of InventoryComp is charged"));
		}
		InventoryComp->S_UseBatteryAmount(-IncreaseAmount);
		UpdateBatteryInfo();
	}
}

void UChargeBatteryWidget::UpdateBatteryInfoDelay()
{
	const FItemData* BatteryData = InventoryComp->GetInventoryItemData(BatteryRow->Name);
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

void UChargeBatteryWidget::PlayVisibleAnimation(bool bIsVisible)
{
	if (bIsVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
		PlayAnimation(FadeIn);
	}
	else
	{
		PlayAnimation(FadeOut);

		FTimerHandle HiddenTimerHandle;
		float HiddenDelay = FadeOut->GetEndTime();
		GetWorld()->GetTimerManager().SetTimer(HiddenTimerHandle, 
			FTimerDelegate::CreateLambda([this]() { SetVisibility(ESlateVisibility::Hidden); 
		}), HiddenDelay, false);
	}
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

void UChargeBatteryWidget::SetEquipBatteryAmount(EChargeBatteryType ChargeBatteryType, int16 Amount)
{
	int8 Percent = 0;
	switch (ChargeBatteryType)
	{
	case EChargeBatteryType::DPV:
		Percent = FMath::RoundToInt((float)Amount / DPVBatteryMax * 100.0f);
		DPVBatteryText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
		break;
	case EChargeBatteryType::NightVisionGoggle:
		Percent = FMath::RoundToInt((float)Amount / NVBatteryMax * 100.0f);
		NVBatteryText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
		break;
	default:
		break;
	}
}

void UChargeBatteryWidget::SetEquipBatteryButtonActivate(EChargeBatteryType ChargeBatteryType, bool bActivate)
{
	switch (ChargeBatteryType)
	{
	case EChargeBatteryType::NightVisionGoggle:
		NVButton->SetIsEnabled(bActivate);
		NVBatteryText->SetVisibility(bActivate ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		break;
	case EChargeBatteryType::DPV:
		DPVButton->SetIsEnabled(bActivate);
		DPVBatteryText->SetVisibility(bActivate ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		break;
	default:
		break;
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



