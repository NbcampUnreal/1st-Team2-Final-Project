// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADTutorialGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/ADTutorialGameState.h"
#include "Framework/ADPlayerController.h"

AADTutorialGameMode::AADTutorialGameMode()
{
    CurrentPhase = ETutorialPhase::None;
}

void AADTutorialGameMode::StartPlay()
{
    Super::StartPlay();

    TutorialPlayerController = Cast<AADPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
    CurrentPhase = ETutorialPhase::Step1_Movement;
    HandleCurrentPhase();
}

void AADTutorialGameMode::AdvanceTutorialPhase()
{
    AADTutorialGameState* TutorialGS = GetGameState<AADTutorialGameState>();
    if (TutorialGS && TutorialGS->GetCurrentPhase() != ETutorialPhase::Complete)
    {
        ETutorialPhase NextPhase = static_cast<ETutorialPhase>(static_cast<uint8>(TutorialGS->GetCurrentPhase()) + 1);

        TutorialGS->SetCurrentPhase(NextPhase);
    }
}

void AADTutorialGameMode::HandleCurrentPhase()
{
    if (!TutorialPlayerController)
    {
        TutorialPlayerController = Cast<AADPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
        if (!TutorialPlayerController) return;
    }

    switch (CurrentPhase)
    {
    case ETutorialPhase::Step1_Movement:
        HandlePhase_Movement(); break;
    case ETutorialPhase::Step2_SprintAndOxygen:
        HandlePhase_Sprint(); break;
    case ETutorialPhase::Step3_Radar:
        HandlePhase_Radar(); break;
    case ETutorialPhase::Step4_Looting:
        HandlePhase_Collecting(); break;
    case ETutorialPhase::Step5_Drone:
        HandlePhase_Drone(); break;
    case ETutorialPhase::Step6_LightToggle:
        HandlePhase_LightToggle(); break;
    case ETutorialPhase::Step7_Items:
        HandlePhase_Items(); break;
    case ETutorialPhase::Step8_OxygenWarning:
        HandlePhase_OxygenWarning(); break;
    case ETutorialPhase::Step9_Revival:
        HandlePhase_Revival(); break;
    case ETutorialPhase::Complete:
        HandlePhase_Complete(); break;
    default:
        break;
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

void AADTutorialGameMode::HandlePhase_Collecting()
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
