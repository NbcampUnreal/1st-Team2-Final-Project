// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADTutorialGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h" 
#include "Tutorial/TutorialStepData.h"
#include "Interactable/OtherActors/TargetIndicators/TargetIndicatorManager.h"
#include "Framework/ADTutorialGameState.h"
#include "Framework/ADPlayerController.h"
#include "EngineUtils.h"

AADTutorialGameMode::AADTutorialGameMode()
{
}

void AADTutorialGameMode::StartPlay()
{
    Super::StartPlay();

    TutorialPlayerController = Cast<AADPlayerController>(UGameplayStatics::GetPlayerController(this, 0));


    if (AADTutorialGameState* TutorialGS = GetGameState<AADTutorialGameState>())
    {
        
    }

}

void AADTutorialGameMode::AdvanceTutorialPhase()
{
    if (AADTutorialGameState* TutorialGS = GetGameState<AADTutorialGameState>())
    {
        ETutorialPhase CurrentPhase = TutorialGS->GetCurrentPhase();
        if (CurrentPhase != ETutorialPhase::Complete)
        {
            ETutorialPhase NextPhase = static_cast<ETutorialPhase>(static_cast<uint8>(CurrentPhase) + 1);
            TutorialGS->SetCurrentPhase(NextPhase);

            HandleCurrentPhase();
        }
    }
}

void AADTutorialGameMode::HandleCurrentPhase()
{

    bIsTypingFinishedForCurrentPhase = false;

	if (!TutorialPlayerController)
	{
		TutorialPlayerController = Cast<AADPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
		if (!TutorialPlayerController) return;
	}

	if (AADTutorialGameState* TutorialGS = GetGameState<AADTutorialGameState>())
	{
		ETutorialPhase CurrentPhase = TutorialGS->GetCurrentPhase();

		switch (CurrentPhase)
		{
		case ETutorialPhase::Step1_Movement:      HandlePhase_Movement(); break;
		case ETutorialPhase::Step2_Sprint:        HandlePhase_Sprint(); break;
		case ETutorialPhase::Step3_Oxygen:        HandlePhase_Oxygen(); break;
		case ETutorialPhase::Step4_Radar:         HandlePhase_Radar(); break;
		case ETutorialPhase::Step5_Looting:       HandlePhase_Looting(); break;
		case ETutorialPhase::Step6_Inventory:     HandlePhase_Inventory(); break;
		case ETutorialPhase::Step7_Drone:         HandlePhase_Drone(); break;
		case ETutorialPhase::Step8_LightToggle:   HandlePhase_LightToggle(); break;
		case ETutorialPhase::Step9_Items:         HandlePhase_Items(); break;
		case ETutorialPhase::Step10_OxygenWarning:HandlePhase_OxygenWarning(); break;
		case ETutorialPhase::Step11_Revival:      HandlePhase_Revival(); break;
		case ETutorialPhase::Complete:            HandlePhase_Complete(); break;

		default:                                  break;
		}

	}
}

void AADTutorialGameMode::HandlePhase_Movement()
{
}

void AADTutorialGameMode::HandlePhase_Sprint()
{
}

void AADTutorialGameMode::HandlePhase_Radar()
{
}

void AADTutorialGameMode::HandlePhase_Oxygen()
{
}

void AADTutorialGameMode::HandlePhase_Looting()
{

    if (!LootableOreClass || !IndicatingTargetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("LootableOreClass or IndicatingTargetClass is not set in TutorialGameMode BP."));
        return;
    }

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), OreSpawnTag, SpawnPoints);

    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find an actor with tag '%s' to spawn the ore."), *OreSpawnTag.ToString());
        return;
    }
    AActor* SpawnPoint = SpawnPoints[0];
    FVector SpawnLocation = SpawnPoint->GetActorLocation();
    FRotator SpawnRotation = SpawnPoint->GetActorRotation();

    AActor* SpawnedOre = GetWorld()->SpawnActor<AActor>(LootableOreClass, SpawnLocation, SpawnRotation);
    if (!SpawnedOre)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn LootableOre."));
        return;
    }

    AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, SpawnLocation, SpawnRotation);
    if (Indicator)
    {

        Indicator->SetupIndicator(SpawnedOre, nullptr); 

        if (ATargetIndicatorManager* Manager = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
        {
            Manager->RegisterNewTarget(Indicator);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("TargetIndicatorManager not found in the world!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn IndicatingTarget."));
        return;
    }
}

void AADTutorialGameMode::HandlePhase_Inventory()
{
}

void AADTutorialGameMode::HandlePhase_Drone()
{
}

void AADTutorialGameMode::HandlePhase_LightToggle()
{
}

void AADTutorialGameMode::HandlePhase_Items()
{
}

void AADTutorialGameMode::HandlePhase_OxygenWarning()
{
}

void AADTutorialGameMode::HandlePhase_Revival()
{
}

void AADTutorialGameMode::HandlePhase_Complete()
{
}

void AADTutorialGameMode::SpawnDownedNPC()
{
}

void AADTutorialGameMode::PlayerActionTriggered(EPlayerActionTrigger ActionType)
{
    if (AADTutorialGameState* TutorialGS = GetGameState<AADTutorialGameState>())
    {
        const ETutorialPhase CurrentPhase = TutorialGS->GetCurrentPhase();
        const FString EnumAsString = UEnum::GetValueAsString(CurrentPhase);
        const FName RowName = FName(*EnumAsString.RightChop(EnumAsString.Find(TEXT("::")) + 2));
        const FTutorialStepData* StepData = TutorialDataTable->FindRow<FTutorialStepData>(RowName, TEXT(""));

        if (StepData && StepData->bWaitForPlayerTrigger)
        {

            if (bIsTypingFinishedForCurrentPhase && StepData->ActionToWaitFor == ActionType)
            {
                AdvanceTutorialPhase();
            }
        }
    }
}

void AADTutorialGameMode::OnTypingAnimationFinished()
{
    bIsTypingFinishedForCurrentPhase = true;
}