// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"

#include "AbyssDiverUnderWorld.h"
#include "StatComponent.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
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
		// StatComponent->OnHealthChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateHealthText);
	}
	if (UOxygenComponent* OxygenComponent = PlayerPawn->FindComponentByClass<UOxygenComponent>())
	{
		OxygenComponent->OnOxygenLevelChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateOxygenText);
		UpdateOxygenText(OxygenComponent->GetMaxOxygenLevel(), OxygenComponent->GetOxygenLevel());
	}
	if (UStaminaComponent* StaminaComponent = PlayerPawn->FindComponentByClass<UStaminaComponent>())
	{
		StaminaComponent->OnStaminaChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateStaminaText);
		UpdateStaminaText(StaminaComponent->GetMaxStamina(), StaminaComponent->GetStamina());
	}
}

void UPlayerHUDWidget::UpdateHealthText(float MaxHealth, float Health)
{
	if (HealthTextBlock)
	{
		FText HealthText = FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(Health), FText::AsNumber(MaxHealth));
		HealthTextBlock->SetText(HealthText);
	}
	else
	{
		LOGV(Warning, TEXT("Health : HealthTextBlock is nullptr"));
	}
}

void UPlayerHUDWidget::UpdateOxygenText(float MaxOxygen, float Oxygen)
{
	if (OxygenTextBlock)
	{
		FText OxygenText = FText::Format(FText::FromString(TEXT("Oxygen : {0} / {1}")), FText::AsNumber(Oxygen), FText::AsNumber(MaxOxygen));
		OxygenTextBlock->SetText(OxygenText);
	}
	else
	{
		LOGV(Warning, TEXT("OxygenTextBlock is nullptr"));
	}
}

void UPlayerHUDWidget::UpdateStaminaText(float MaxStamina, float Stamina)
{
	if (StaminaTextBlock)
	{
		FText StaminaText = FText::Format(FText::FromString(TEXT("Stamina : {0} / {1}")), FText::AsNumber(Stamina), FText::AsNumber(MaxStamina));
		StaminaTextBlock->SetText(StaminaText);
	}
	else
	{
		LOGV(Warning, TEXT("StaminaTextBlock is nullptr"));
	}
}
