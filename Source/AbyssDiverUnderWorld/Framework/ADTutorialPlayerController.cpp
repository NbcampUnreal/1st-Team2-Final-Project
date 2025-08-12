#include "Framework/ADTutorialPlayerController.h"
#include "Character/UnderwaterCharacter.h"
#include "EnhancedInputComponent.h"
#include "Tutorial/TutorialManager.h"
#include "Framework/ADTutorialGameMode.h"
#include "Framework/ADTutorialGameState.h"
#include "Kismet/GameplayStatics.h"

AADTutorialPlayerController::AADTutorialPlayerController()
{
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
			if (BatteryAction)
			{
				EnhancedInput->BindAction(BatteryAction, ETriggerEvent::Started, this, &AADTutorialPlayerController::OnBatteryStarted);
				EnhancedInput->BindAction(BatteryAction, ETriggerEvent::Completed, this, &AADTutorialPlayerController::OnBatteryCompleted);
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
		}
	}
}

void AADTutorialPlayerController::CheckTutorialObjective(const FInputActionValue& Value, UInputAction* SourceAction)
{
	AUnderwaterCharacter* PossessedCharacter = Cast<AUnderwaterCharacter>(GetPawn());
	if (!PossessedCharacter) return;

	AADTutorialGameState* TutorialGS = GetWorld() ? GetWorld()->GetGameState<AADTutorialGameState>() : nullptr;
	if (!TutorialGS) return;

	ETutorialPhase CurrentPhase = TutorialGS->GetCurrentPhase();
	bool bObjectiveMet = false;

	if (SourceAction == InteractAction)
	{
		if (CurrentPhase == ETutorialPhase::Step7_Drone ||
			CurrentPhase == ETutorialPhase::Step15_Resurrection )
		{
			bObjectiveMet = true;
		}
	}

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
	ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
	if (Manager && Manager->IsGaugeObjectiveActive())
	{
		Manager->NotifyInteractionStart();
	}
	else
	{
		CheckTutorialObjective(Value, InteractAction);
	}
}

void AADTutorialPlayerController::OnSprintStarted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionStart();
	}
}

void AADTutorialPlayerController::OnRadarStarted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionStart();
	}
}

void AADTutorialPlayerController::OnInventoryStarted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionStart();
	}
}

void AADTutorialPlayerController::OnLightToggleStarted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionStart();
	}
}

void AADTutorialPlayerController::OnBatteryStarted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionStart();
	}
}

void AADTutorialPlayerController::OnDropStarted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionStart();
	}
}

void AADTutorialPlayerController::OnReviveStarted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionStart();
	}
}

void AADTutorialPlayerController::OnInteractCompleted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnSprintCompleted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnRadarCompleted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnInventoryCompleted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnLightToggleCompleted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnBatteryCompleted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnDropCompleted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionEnd();
	}
}

void AADTutorialPlayerController::OnReviveCompleted(const FInputActionValue& Value)
{
	if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Manager->NotifyInteractionEnd();
	}
}
