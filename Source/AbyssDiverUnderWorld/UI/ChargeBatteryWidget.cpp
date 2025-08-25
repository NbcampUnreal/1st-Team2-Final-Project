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
#include "Tutorial/TutorialManager.h"
#include "Components/AudioComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Framework/ADTutorialGameMode.h"


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

	bChargeBatteryWidgetShowed = false;

	float InitializeRepeatDelay = 2.0f;
	GetWorld()->GetTimerManager().SetTimer(InitialzieTimerHandle, this, &UChargeBatteryWidget::InitializeChargeBatteryWidget, InitializeRepeatDelay, true);
}

void UChargeBatteryWidget::NativeDestruct()
{
	Super::NativeDestruct();
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	LOGB(Warning, TEXT("ChargeBatteryWidget Destruct"));
}

void UChargeBatteryWidget::StartChargeBattery(FName ItemName)
{
	CurrentChargeItem = ItemName;

	if (SoundCue)
	{
		UAudioComponent* Sound = UGameplayStatics::SpawnSound2D(this, SoundCue, 1.0f, 1.0f, 0.0f, nullptr, true);
		if (Sound) { Sound->Play(); ChargeBatterySound = Sound; }
	}

	if (AADTutorialGameMode* GM = GetWorld()->GetAuthGameMode<AADTutorialGameMode>())
	{
		if (GM->BatteryStartPercentOverride < 0.f)
		{
			int32 MaxToCompare = (CurrentChargeItem == DPVRow->Name) ? DPVBatteryMax : NVBatteryMax;

			int32 CurrentAmount = 0;
			if (InventoryComp && EquipUseComp)
			{
				if (FItemData* CurEquip = InventoryComp->GetCurrentEquipmentItemData();
					CurEquip && CurEquip->Name == CurrentChargeItem)
				{
					CurrentAmount = EquipUseComp->Amount;
				}
				else
				{
					if (const FItemData* ItemInfo = InventoryComp->GetInventoryItemData(CurrentChargeItem))
						CurrentAmount = ItemInfo->Amount;
				}
			}

			float StartPct = (MaxToCompare > 0) ? (float)CurrentAmount / (float)MaxToCompare * 100.f : 0.f;
			GM->BatteryStartPercentOverride = FMath::Clamp(StartPct, 0.f, 100.f);
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
		if (ChargeBatterySound && ChargeBatterySound->IsPlaying())
		{
			ChargeBatterySound->Stop();
		}
		return false;
	}

	const FItemData* BatteryInfo = InventoryComp->GetInventoryItemData(BatteryRow->Name);
	if (BatteryInfo->Quantity > 0)
	{
		LOGB(Warning, TEXT("The battery can be charged"));
		return true;
	}

	if (ChargeBatterySound && ChargeBatterySound->IsPlaying())
	{
		ChargeBatterySound->Stop();
	}
	LOGB(Warning, TEXT("There is no battery."));
	return false;
}

void UChargeBatteryWidget::ChargeBatteryAmount()
{
	if (!CanCharge()) return;
	if (!(InventoryComp && EquipUseComp)) return;
	if (CurrentChargeItem == NAME_None) return;

	const int32 MaxToCompare = (CurrentChargeItem == DPVRow->Name) ? DPVBatteryMax : NVBatteryMax;

	const float ChargeRate = 0.01f;
	const int32 TickIncrease = FMath::Max(1, FMath::RoundToInt(MaxToCompare * ChargeRate));

	int32 BeforeAmount = 0;
	bool bChargingEquipped = false;

	if (FItemData* CurEquip = InventoryComp->GetCurrentEquipmentItemData();
		CurEquip && CurEquip->Name == CurrentChargeItem)
	{
		bChargingEquipped = true;
		BeforeAmount = EquipUseComp->Amount;
	}
	else
	{
		if (const FItemData* ItemInfo = InventoryComp->GetInventoryItemData(CurrentChargeItem))
			BeforeAmount = ItemInfo->Amount;
	}

	if (BeforeAmount >= MaxToCompare)
	{
		StopChargeBattery();
		return;
	}

	const int32 Room = MaxToCompare - BeforeAmount;
	const int32 AppliedIncrease = FMath::Clamp(TickIncrease, 0, Room);

	if (bChargingEquipped)
	{
		EquipUseComp->S_IncreaseAmount(AppliedIncrease);
		LOGB(Warning, TEXT("Amount of EquipUseComp is charged"));
	}
	else
	{
		InventoryComp->S_EquipmentChargeBattery(ChargeBatteryTypeMap[CurrentChargeItem], AppliedIncrease);
		LOGB(Warning, TEXT("Amount of InventoryComp is charged"));
	}

	InventoryComp->S_UseBatteryAmount(-AppliedIncrease);
	UpdateBatteryInfo();

	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		const float DeltaPercent = (MaxToCompare > 0) ? (float)AppliedIncrease / (float)MaxToCompare * 100.f : 0.f;
		if (DeltaPercent > KINDA_SMALL_NUMBER)
			Manager->AddGaugeProgress(DeltaPercent);
	}

	if (AppliedIncrease >= Room)
	{
		StopChargeBattery();
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
		if (!bChargeBatteryWidgetShowed)
		{
			GetWorld()->GetTimerManager().ClearTimer(HiddenTimerHandle);
			SetVisibility(ESlateVisibility::Visible);
			bChargeBatteryWidgetShowed = true;
			if (FadeIn && !IsAnimationPlaying(FadeIn))
				PlayAnimation(FadeIn);
		}
	}
	else
	{
		if (bChargeBatteryWidgetShowed)
		{
			if (FadeOut && !IsAnimationPlaying(FadeOut))
				PlayAnimation(FadeOut);

			float HiddenDelay = FadeOut->GetEndTime();
			GetWorld()->GetTimerManager().SetTimer(HiddenTimerHandle, 
				FTimerDelegate::CreateLambda([this]() { 
					SetVisibility(ESlateVisibility::Hidden); 
					bChargeBatteryWidgetShowed = false;
			}), HiddenDelay, false);
		}
	}
}

void UChargeBatteryWidget::InitializeChargeBatteryWidget()
{
	if (InitializeNum <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(InitialzieTimerHandle);
		return;
	}
	InitializeNum--;
	if (InventoryComp == nullptr)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AADPlayerState* PS = Cast<AADPlayerState>(PC->PlayerState);
			if (PS)
			{
				InventoryComp = PS->GetInventory();
				if (InventoryComp)
				{
					InventoryComp->SetChargeBatteryInstance(this);
				}
			}
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



