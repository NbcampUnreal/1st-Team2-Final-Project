// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADTutorialGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h" 
#include "Tutorial/TutorialStepData.h"
#include "Interactable/OtherActors/TargetIndicators/TargetIndicatorManager.h"
#include "Components/PrimitiveComponent.h"
#include "Framework/ADTutorialGameState.h"
#include "Framework/ADPlayerController.h"
#include "Engine/Light.h"
#include "Components/LightComponent.h" 
#include "EngineUtils.h"

AADTutorialGameMode::AADTutorialGameMode()
{
}

// AADTutorialGameMode.cpp

void AADTutorialGameMode::StartPlay()
{
    Super::StartPlay();

    TutorialPlayerController = Cast<AADPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

    TArray<AActor*> TutorialActors;
    const FName RadarTag = FName("Radar");

    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialRock_D2"), TutorialActors);
    for (AActor* Actor : TutorialActors)
    {
        if (IsValid(Actor))
        {
            Actor->Tags.Remove(RadarTag);
            Actor->SetActorHiddenInGame(true);
        }
    }
    TutorialActors.Empty();

    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialMonster_D2"), TutorialActors);
    for (AActor* Actor : TutorialActors)
    {
        if (IsValid(Actor))
        {
            Actor->Tags.Remove(RadarTag);
            Actor->SetActorHiddenInGame(true); 
        }
    }
    TutorialActors.Empty();

    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialFriendly_D2"), TutorialActors);
    for (AActor* Actor : TutorialActors)
    {
        if (IsValid(Actor))
        {
            Actor->Tags.Remove(RadarTag);
            Actor->SetActorHiddenInGame(true); 
        }
    }
    TutorialActors.Empty();

    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialDrone"), TutorialActors);
    for (AActor* Actor : TutorialActors)
    {
        if (IsValid(Actor))
        {
            Actor->Tags.Remove(RadarTag);
            Actor->SetActorHiddenInGame(true); 
        }
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
    HidePhaseActors();
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
        case ETutorialPhase::Dialogue_02:         HandlePhase_Dialogue_02(); break;
        case ETutorialPhase::Step5_Looting:       HandlePhase_Looting(); break;
        case ETutorialPhase::Step6_Inventory:     HandlePhase_Inventory(); break;
        case ETutorialPhase::Step7_Drone:         HandlePhase_Drone(); break;
        case ETutorialPhase::Step8_LightToggle:   HandlePhase_LightToggle(); break;
        case ETutorialPhase::Step9_Items:         HandlePhase_Items(); break;
        case ETutorialPhase::Step10_Battery:      HandlePhase_Battery(); break;
        case ETutorialPhase::Step11_Drop:         HandlePhase_Drop(); break;
        case ETutorialPhase::Step12_OxygenWarning:HandlePhase_OxygenWarning(); break;
        case ETutorialPhase::Step13_Revive:       HandlePhase_Revive(); break;
        case ETutorialPhase::Dialogue_04:         break;
        case ETutorialPhase::Step14_Die:          HandlePhase_Die(); break;
        case ETutorialPhase::Step15_Resurrection: HandlePhase_Resurrection(); break;
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

void AADTutorialGameMode::HandlePhase_Dialogue_02()
{
    if (!IndicatingTargetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("IndicatingTargetClass is not set in TutorialGameMode BP."));
        return;
    }

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), DialogueTargetSpawnTag, SpawnPoints);
    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find an actor with tag '%s' to spawn the dialogue target."), *DialogueTargetSpawnTag.ToString());
        return;
    }

    AActor* SpawnPoint = SpawnPoints[0];
    FVector   SpawnLocation = SpawnPoint->GetActorLocation();
    FRotator  SpawnRotation = SpawnPoint->GetActorRotation();

    AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, SpawnLocation, SpawnRotation);
    if (Indicator)
    {
        Indicator->SetupIndicator(nullptr, DialogueIndicatorIcon);

        if (ATargetIndicatorManager* Manager = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
        {
            Manager->RegisterNewTarget(Indicator);
        }
        TrackPhaseActor(Indicator);
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialRock_D2"), FoundActors);
    for (AActor* Rock : FoundActors)
    {
        Rock->SetActorHiddenInGame(false);
        Rock->Tags.Add(FName("Radar"));
    }
    FoundActors.Empty();

    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialMonster_D2"), FoundActors);
    for (AActor* Monster : FoundActors)
    {
        Monster->SetActorHiddenInGame(false);
        Monster->Tags.Add(FName("Radar"));
        TrackPhaseActor(Monster); 
    }
    FoundActors.Empty();

    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialFriendly_D2"), FoundActors);
    for (AActor* Friendly : FoundActors)
    {
        Friendly->SetActorHiddenInGame(false);
        Friendly->Tags.Add(FName("Radar"));
        TrackPhaseActor(Friendly); 
    }

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
    FVector  SpawnLocation = SpawnPoint->GetActorLocation();
    FRotator SpawnRotation = SpawnPoint->GetActorRotation();

    AActor* SpawnedOre = GetWorld()->SpawnActor<AActor>(LootableOreClass, SpawnLocation, SpawnRotation);
    if (!SpawnedOre)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn LootableOre."));
        return;
    }
    TrackPhaseActor(SpawnedOre);

    AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, SpawnLocation, SpawnRotation);
    if (Indicator)
    {
        Indicator->SetupIndicator(SpawnedOre, LootingOreIcon);
        if (ATargetIndicatorManager* Manager = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
        {
            Manager->RegisterNewTarget(Indicator);
        }
        BindIndicatorToOwner(SpawnedOre, Indicator);
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
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialDrone"), FoundActors);
    if (FoundActors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot find actor with tag 'TutorialDrone'"));
        return;
    }

    AActor* Drone = FoundActors[0];
    Drone->SetActorHiddenInGame(false);
    Drone->Tags.Add(FName("Radar"));
    TrackPhaseActor(Drone);

    if (IndicatingTargetClass)
    {
        AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, Drone->GetActorLocation(), Drone->GetActorRotation());
        if (Indicator)
        {
            Indicator->SetupIndicator(Drone, DroneIndicatorIcon);
            if (ATargetIndicatorManager* Manager = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
            {
                Manager->RegisterNewTarget(Indicator);
            }
            BindIndicatorToOwner(Drone, Indicator);
        }
    }
}

void AADTutorialGameMode::HandlePhase_LightToggle()
{
    DisabledLights.Empty();

    for (TActorIterator<ALight> It(GetWorld()); It; ++It)
    {
        ALight* LightActor = *It;
        if (LightActor)
        {
            ULightComponent* LightComponent = LightActor->GetLightComponent();
            if (LightComponent && LightComponent->IsVisible())
            {
                DisabledLights.Add(LightActor);
                LightComponent->SetVisibility(false);
            }
        }
    }
}

void AADTutorialGameMode::HandlePhase_Items()
{
    ItemsPhaseProgress = 0;
}

void AADTutorialGameMode::HandlePhase_Battery()
{
}

void AADTutorialGameMode::HandlePhase_Drop()
{
}

void AADTutorialGameMode::HandlePhase_OxygenWarning()
{
}

void AADTutorialGameMode::HandlePhase_Revive()
{
}

void AADTutorialGameMode::HandlePhase_Die()
{
}

void AADTutorialGameMode::HandlePhase_Resurrection()
{
}

void AADTutorialGameMode::HandlePhase_Complete()
{
}

void AADTutorialGameMode::SpawnDownedNPC()
{
}

void AADTutorialGameMode::OnTypingAnimationFinished()
{
    bIsTypingFinishedForCurrentPhase = true;
}

void AADTutorialGameMode::HidePhaseActors()
{
    for (AActor* ActorToDestroy : ActorsToShowThisPhase)
    {
        if (ActorToDestroy) 
        {
            ActorToDestroy->OnDestroyed.RemoveAll(this);
            ActorToDestroy->Destroy();
        }
    }
    ActorsToShowThisPhase.Empty();
}

void AADTutorialGameMode::OnPlayerItemAction(EPlayerActionTrigger ItemActionType)
{
    if (AADTutorialGameState* TutorialGS = GetGameState<AADTutorialGameState>())
    {
        if (TutorialGS->GetCurrentPhase() != ETutorialPhase::Step9_Items)
        {
            return;
        }
    }

    if (ItemsPhaseProgress == 0 && ItemActionType == EPlayerActionTrigger::UseItem1)
    {
        ItemsPhaseProgress++; 
    }

    else if (ItemsPhaseProgress == 1 && ItemActionType == EPlayerActionTrigger::UseItem2)
    {
        ItemsPhaseProgress++;
    }

    else if (ItemsPhaseProgress == 2 && ItemActionType == EPlayerActionTrigger::UseItem3)
    {
        AdvanceTutorialPhase();
    }
}

void AADTutorialGameMode::BindIndicatorToOwner(AActor* OwnerActor, AActor* IndicatorActor)
{
    if (!IsValid(OwnerActor) || !IsValid(IndicatorActor)) return;

    OwnerToIndicator.Add(OwnerActor, IndicatorActor);
    OwnerActor->OnDestroyed.AddDynamic(this, &AADTutorialGameMode::OnTrackedOwnerDestroyed);
}

void AADTutorialGameMode::OnTrackedOwnerDestroyed(AActor* DestroyedActor)
{
    if (TWeakObjectPtr<AActor>* FoundIndicator = OwnerToIndicator.Find(DestroyedActor))
    {
        if (FoundIndicator->IsValid())
        {
            FoundIndicator->Get()->Destroy();
        }
        OwnerToIndicator.Remove(DestroyedActor);
    }
}