// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChargeBatteryWidget.generated.h"

struct FItemData;
class URichTextBlock;
class UADInventoryComponent;
class UEquipUseComponent;
class UButton;
class UNoticeWidget;
struct FFADItemDataRow;

UENUM()
enum class EChargeBatteryType
{
	NightVisionGoggle = 2,
	DPV = 3,
	Battery = 7,
	MAX = -1
};

#define LOGB(Verbosity, Format, ...) UE_LOG(InventoryLog, Verbosity, TEXT("%s(%s) %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(BatteryLog, Log, All);

UCLASS()
class ABYSSDIVERUNDERWORLD_API UChargeBatteryWidget : public UUserWidget
{
	GENERATED_BODY()
	
#pragma region Method
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UFUNCTION(BlueprintCallable)
	void StartChargeBattery(FName ItemName);
	UFUNCTION(BlueprintCallable)
	void StopChargeBattery();

	UFUNCTION(BlueprintCallable)
	bool CanCharge();

	bool HasBattery();

	void ChargeBatteryAmount();

	void UpdateBatteryInfoDelay();
	void UpdateBatteryInfo();

	void PlayVisibleAnimation(bool bIsVisible);
private:
	void InitializeChargeBatteryWidget();
	uint8 bChargeBatteryWidgetShowed : 1;
#pragma endregion

#pragma region Variable
public:
	FFADItemDataRow* DPVRow;
	FFADItemDataRow* NVRow;
	FFADItemDataRow* BatteryRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> SoundCue = nullptr;
	UPROPERTY()
	TObjectPtr<UAudioComponent> ChargeBatterySound = nullptr;
private:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeIn;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeOut;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URichTextBlock> BatteryAmountText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> BatteryNumText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> DPVBatteryText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> NVBatteryText; 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DPVButton; 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NVButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNoticeWidget> NoticeWidget;

	UPROPERTY()
	TObjectPtr<UADInventoryComponent> InventoryComp = nullptr;
	UPROPERTY()
	TObjectPtr<UEquipUseComponent> EquipUseComp = nullptr;

	int32 DPVBatteryMax = 0;
	int32 NVBatteryMax = 0;
	uint8 BatteryMax = 0;
	uint8 InitializeNum = 5;

	FName CurrentChargeItem;
	FTimerHandle IncreaseTimerHandle;
	FTimerHandle InitialzieTimerHandle;
	FTimerHandle HiddenTimerHandle;

	TMap<FName, EChargeBatteryType> ChargeBatteryTypeMap = {
		{FName(TEXT("NightVisionGoggle")), EChargeBatteryType::NightVisionGoggle},
		{FName(TEXT("DPV")), EChargeBatteryType::DPV},
		{FName(TEXT("Battery")), EChargeBatteryType::Battery}
	};
#pragma endregion

#pragma region Getter/Setter
public:
	void SetEquipBatteryAmount(EChargeBatteryType ChargeBatteryType, int16 Amount);
	void SetEquipBatteryButtonActivate(EChargeBatteryType ChargeBatteryType, bool bActivate);

#pragma endregion
};
