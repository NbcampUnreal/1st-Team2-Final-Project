// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "StatComponent.h"
#include "Components/RichTextBlock.h"
#include "PlayerComponent/OxygenComponent.h"
#include "PlayerComponent/StaminaComponent.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// HUD Widget 자체가 Bind를 진행할 수는 있긴 하다.
	// 하지만, Respawn 시에 다시 바인딩을 해야 하기 때문에 어차피 BindWidget 함수가 필요하다.
}

void UPlayerHUDWidget::BindWidget(APawn* PlayerPawn)
{
	// 아직, Pawn 이 없을 경우 BindWidget을 수행하지 않는다.
	if (!PlayerPawn)
	{
		return;
	}

	// Bind 을 외부에서 하는 것을 고려할 것. UI와 Data Component의 의존성을 줄일 필요가 있어 보인다.
	if (UStatComponent* StatComponent = PlayerPawn->FindComponentByClass<UStatComponent>())
	{
		StatComponent->OnHealthChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateHealthText);
		UpdateHealthText(StatComponent->GetCurrentHealth(), StatComponent->GetMaxHealth());
	}
	if (UOxygenComponent* OxygenComponent = PlayerPawn->FindComponentByClass<UOxygenComponent>())
	{
		OxygenComponent->OnOxygenLevelChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateOxygenText);
		UpdateOxygenText(OxygenComponent->GetOxygenLevel(), OxygenComponent->GetMaxOxygenLevel());
	}
	if (UStaminaComponent* StaminaComponent = PlayerPawn->FindComponentByClass<UStaminaComponent>())
	{
		StaminaComponent->OnStaminaChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateStaminaText);
		UpdateStaminaText(StaminaComponent->GetStamina(), StaminaComponent->GetMaxStamina());
	}
}

void UPlayerHUDWidget::UpdateHealthText(int32 Health, int32 MaxHealth)
{
	if (HealthTextBlock)
	{
		FText HealthText = FText::Format(FText::FromString(TEXT("Health : {0} / {1}")), FText::AsNumber(Health), FText::AsNumber(MaxHealth));
		HealthTextBlock->SetText(HealthText);
	}
	else
	{
		LOGV(Warning, TEXT("Health : HealthTextBlock is nullptr"));
	}
}

void UPlayerHUDWidget::UpdateOxygenText(float Oxygen, float MaxOxygen)
{
	if (OxygenTextBlock)
	{
		FNumberFormattingOptions FormatOptions;
		FormatOptions.MinimumFractionalDigits = 2;
		FormatOptions.MaximumFractionalDigits = 2;
		
		FText OxygenText = FText::Format(
			FText::FromString(TEXT("Oxygen : {0} / {1}")),
			FText::AsNumber(Oxygen, &FormatOptions),
			FText::AsNumber(MaxOxygen, &FormatOptions))
		;
		OxygenTextBlock->SetText(OxygenText);
	}
	else
	{
		LOGV(Warning, TEXT("OxygenTextBlock is nullptr"));
	}
}

void UPlayerHUDWidget::UpdateStaminaText(float Stamina, float MaxStamina)
{
	if (StaminaTextBlock)
	{
		FNumberFormattingOptions FormatOptions;
		FormatOptions.MinimumFractionalDigits = 2;
		FormatOptions.MaximumFractionalDigits = 2;
		
		FText StaminaText = FText::Format(
			FText::FromString(TEXT("Stamina : {0} / {1}")),
			FText::AsNumber(Stamina, &FormatOptions),
			FText::AsNumber(MaxStamina, &FormatOptions))
		;
		StaminaTextBlock->SetText(StaminaText);
	}
	else
	{
		LOGV(Warning, TEXT("StaminaTextBlock is nullptr"));
	}
}
