﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDComponent.h"

#include "AbyssDiverUnderWorld.h"
#include "Character/PlayerHUDWidget.h"
#include "Framework/CreateTeamWidget.h"

UPlayerHUDComponent::UPlayerHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController)
	{
		LOGV(Warning, TEXT("PlayerController is nullptr"));
		return;
	}
	// Local Controller일 경우에만 HudWidget을 생성합니다.
	if (!PlayerController->IsLocalController())
	{
		return;
	}

	if (HudWidgetClass)
	{
		HudWidget = CreateWidget<UPlayerHUDWidget>(PlayerController, HudWidgetClass);
		if (HudWidget)
		{
			APawn* PlayerPawn = PlayerController->GetPawn();
			HudWidget->AddToViewport();
			HudWidget->BindWidget(PlayerPawn);
		}
		else
		{
			LOGV(Warning, TEXT("Failed to create HudWidget"));
		}
	}
	else
	{
		LOGV(Warning, TEXT("HudWidgetClass is nullptr"));
	}

	// Pawn이 늦게 생성이 되거나 혹은, Respawn 상황에도 Binding을 수행해야 한다.
	PlayerController->OnPossessedPawnChanged.AddDynamic(this, &UPlayerHUDComponent::OnPossessedPawnChanged);
}

void UPlayerHUDComponent::SetVisibility(const bool NewVisible) const
{
	if (HudWidget)
	{
		HudWidget->SetVisibility(NewVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	LOGN(TEXT("Rebind HUD Widget"))
	
	// OnPossessedPawnChanged에서는 새로운 Pawn Possess 상황만 대응한다.
	// 사망 시의 UI는 Character Component의 사망 시점에서 처리하도록 한다.
	if (NewPawn)
	{
		if (HudWidget)
		{
			HudWidget->BindWidget(NewPawn);
		}
		else
		{
			LOGV(Warning, TEXT("HudWidget is nullptr when possessed"));
		}
	}
}
