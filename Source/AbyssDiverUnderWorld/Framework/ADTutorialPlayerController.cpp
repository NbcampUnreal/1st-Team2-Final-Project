// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/ADTutorialPlayerController.h"
#include "Character/UnderwaterCharacter.h"
#include "EnhancedInputComponent.h"
#include "Tutorial/TutorialManager.h"
#include "Framework/ADTutorialGameMode.h"
#include "Framework/ADTutorialGameState.h"
#include "Kismet/GameplayStatics.h"

AADTutorialPlayerController::AADTutorialPlayerController()
	: Super()
{
	
}

void AADTutorialPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CachedTutorialManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
	if (!CachedTutorialManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("AADTutorialPlayerController: ATutorialManager not found in the world. Tutorial features may not work."));
	}
}

void AADTutorialPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (AUnderwaterCharacter* PossessedCharacter = Cast<AUnderwaterCharacter>(InPawn))
	{
		if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
		{
			if (PossessedCharacter->GetSprintAction())
			{
				EnhancedInput->BindAction(PossessedCharacter->GetSprintAction(), ETriggerEvent::Started, this, &AADTutorialPlayerController::OnSprintStarted);
				EnhancedInput->BindAction(PossessedCharacter->GetSprintAction(), ETriggerEvent::Completed, this, &AADTutorialPlayerController::OnSprintCompleted);
			}
			if (PossessedCharacter->GetRadarAction())
			{
				EnhancedInput->BindAction(PossessedCharacter->GetRadarAction(), ETriggerEvent::Started, this, &AADTutorialPlayerController::OnRadarStarted);
				EnhancedInput->BindAction(PossessedCharacter->GetRadarAction(), ETriggerEvent::Completed, this, &AADTutorialPlayerController::OnRadarCompleted);
			}
			if (PossessedCharacter->GetLightToggleAction())
			{
				EnhancedInput->BindAction(PossessedCharacter->GetLightToggleAction(), ETriggerEvent::Started, this, &AADTutorialPlayerController::OnLightToggleStarted);
				EnhancedInput->BindAction(PossessedCharacter->GetLightToggleAction(), ETriggerEvent::Completed, this, &AADTutorialPlayerController::OnLightToggleCompleted);
			}
			if (InteractAction)
			{
				EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AADTutorialPlayerController::OnInteractStarted);
				EnhancedInput->BindAction(InteractAction, ETriggerEvent::Completed, this, &AADTutorialPlayerController::OnInteractCompleted);
			}
			if (InventoryAction)
			{
				EnhancedInput->BindAction(InventoryAction, ETriggerEvent::Started, this, &AADTutorialPlayerController::OnInventoryStarted);
				EnhancedInput->BindAction(InventoryAction, ETriggerEvent::Completed, this, &AADTutorialPlayerController::OnInventoryCompleted);
			}

			if (DropAction)
			{
				EnhancedInput->BindAction(DropAction, ETriggerEvent::Started, this, &AADTutorialPlayerController::OnDropStarted);
				EnhancedInput->BindAction(DropAction, ETriggerEvent::Completed, this, &AADTutorialPlayerController::OnDropCompleted);
			}
			if (ReviveAction)
			{
				EnhancedInput->BindAction(ReviveAction, ETriggerEvent::Started, this, &AADTutorialPlayerController::OnReviveStarted);
				EnhancedInput->BindAction(ReviveAction, ETriggerEvent::Completed, this, &AADTutorialPlayerController::OnReviveCompleted);
			}
			if (UInputAction* Item1Action = PossessedCharacter->GetSelectInventorySlot1())
			{
				EnhancedInput->BindAction(Item1Action, ETriggerEvent::Triggered, this, &AADTutorialPlayerController::OnUseItem1);
			}
			if (UInputAction* Item2Action = PossessedCharacter->GetSelectInventorySlot2())
			{
				EnhancedInput->BindAction(Item2Action, ETriggerEvent::Triggered, this, &AADTutorialPlayerController::OnUseItem2);
			}
			if (UInputAction* Item3Action = PossessedCharacter->GetSelectInventorySlot3())
			{
				EnhancedInput->BindAction(Item3Action, ETriggerEvent::Triggered, this, &AADTutorialPlayerController::OnUseItem3);
			}
		}
	}
}

void AADTutorialPlayerController::CheckTutorialObjective(const FInputActionValue& Value, UInputAction* SourceAction)
{
	AADTutorialGameState* TutorialGS = GetWorld() ? GetWorld()->GetGameState<AADTutorialGameState>() : nullptr;
	if (!TutorialGS) return;

	ETutorialPhase CurrentPhase = TutorialGS->GetCurrentPhase();
	bool bObjectiveMet = false;

	if (bObjectiveMet)
	{
		if (AADTutorialGameMode* GM = GetWorld()->GetAuthGameMode<AADTutorialGameMode>())
		{
			if (GM->IsTypingFinishedForCurrentPhase())
			{
				RequestAdvanceTutorialPhase();
			}
		}
	}
}

void AADTutorialPlayerController::RequestAdvanceTutorialPhase()
{
	if (AADTutorialGameMode* GM = GetWorld()->GetAuthGameMode<AADTutorialGameMode>())
	{
		GM->AdvanceTutorialPhase();
	}
}

void AADTutorialPlayerController::ReportItemAction(EPlayerActionTrigger ItemActionType)
{
	if (AADTutorialGameMode* GM = GetWorld()->GetAuthGameMode<AADTutorialGameMode>())
	{
		GM->OnPlayerItemAction(ItemActionType);
	}
}

void AADTutorialPlayerController::OnInteractStarted(const FInputActionValue& Value)
{
	if (!IsValid(CachedTutorialManager)) return;

	AADTutorialGameState* GS = GetWorld()->GetGameState<AADTutorialGameState>();
	if (GS)
	{
		const ETutorialPhase Current = GS->GetCurrentPhase();

		if (Current == ETutorialPhase::Step13_Revive
			|| Current == ETutorialPhase::Step14_Die
			|| Current == ETutorialPhase::Step11_Drop)
		{
			CachedTutorialManager->NotifyInteractionStart();
			return; 
		}
	}

	CheckTutorialObjective(Value, InteractAction);
}

void AADTutorialPlayerController::OnInteractCompleted(const FInputActionValue& Value)
{
	if (IsValid(CachedTutorialManager))
	{
		CachedTutorialManager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnSprintStarted(const FInputActionValue& Value)
{
	AADTutorialGameState* GS = GetWorld()->GetGameState<AADTutorialGameState>();

	if (GS && GS->GetCurrentPhase() == ETutorialPhase::Step2_Sprint)
	{
		if (IsValid(CachedTutorialManager))
		{
			CachedTutorialManager->NotifyInteractionStart();
		}
	}
}

void AADTutorialPlayerController::OnSprintCompleted(const FInputActionValue& Value)
{
	if (IsValid(CachedTutorialManager))
	{
		CachedTutorialManager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnRadarStarted(const FInputActionValue& Value)
{
	AADTutorialGameState* GS = GetWorld()->GetGameState<AADTutorialGameState>();

	if (GS && GS->GetCurrentPhase() == ETutorialPhase::Step4_Radar)
	{
		if (IsValid(CachedTutorialManager))
		{
			CachedTutorialManager->NotifyInteractionStart();
		}
	}
}

void AADTutorialPlayerController::OnRadarCompleted(const FInputActionValue& Value)
{
	if (IsValid(CachedTutorialManager))
	{
		CachedTutorialManager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnInventoryStarted(const FInputActionValue& Value)
{
	if (IsValid(CachedTutorialManager))
	{
		CachedTutorialManager->OnInventoryInputPressed();

		AADTutorialGameState* GS = GetWorld()->GetGameState<AADTutorialGameState>();

		if (GS && GS->GetCurrentPhase() == ETutorialPhase::Step6_Inventory)
		{
			CachedTutorialManager->NotifyInteractionStart();
		}
	}
}

void AADTutorialPlayerController::OnInventoryCompleted(const FInputActionValue& Value)
{
	if (IsValid(CachedTutorialManager))
	{
		CachedTutorialManager->OnInventoryInputReleased();
		CachedTutorialManager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnLightToggleStarted(const FInputActionValue& Value)
{
	AADTutorialGameState* GS = GetWorld()->GetGameState<AADTutorialGameState>();

	if (GS && GS->GetCurrentPhase() == ETutorialPhase::Step8_LightToggle)
	{
		if (IsValid(CachedTutorialManager))
		{
			CachedTutorialManager->NotifyInteractionStart();
		}
	}
}

void AADTutorialPlayerController::OnLightToggleCompleted(const FInputActionValue& Value)
{
	if (IsValid(CachedTutorialManager))
	{
		CachedTutorialManager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnBatteryStarted(const FInputActionValue& Value)
{
}

void AADTutorialPlayerController::OnBatteryCompleted(const FInputActionValue& Value)
{
}

void AADTutorialPlayerController::OnDropStarted(const FInputActionValue& Value)
{
}

void AADTutorialPlayerController::OnDropCompleted(const FInputActionValue& Value)
{
}

void AADTutorialPlayerController::OnReviveStarted(const FInputActionValue& Value)
{
}

void AADTutorialPlayerController::OnReviveCompleted(const FInputActionValue& Value)
{
	if (IsValid(CachedTutorialManager))
	{
		CachedTutorialManager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnUseItem1(const FInputActionValue& Value) 
{ 
	ReportItemAction(EPlayerActionTrigger::UseItem1); 
}

void AADTutorialPlayerController::OnUseItem2(const FInputActionValue& Value) 
{ 
	ReportItemAction(EPlayerActionTrigger::UseItem2); 
}

void AADTutorialPlayerController::OnUseItem3(const FInputActionValue& Value) 
{ 
	ReportItemAction(EPlayerActionTrigger::UseItem3); 
}