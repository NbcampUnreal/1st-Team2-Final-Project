// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADTutorialGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h" 
#include "Tutorial/TutorialStepData.h"
#include "Tutorial/TutorialManager.h"
#include "Interactable/OtherActors/TargetIndicators/TargetIndicatorManager.h"
#include "Components/PrimitiveComponent.h"
#include "Framework/ADTutorialGameState.h"
#include "Framework/ADPlayerController.h"
#include "Engine/Light.h"
#include "NiagaraComponent.h"
#include "Components/LightComponent.h" 
#include "EngineUtils.h"

AADTutorialGameMode::AADTutorialGameMode()
{
}

void AADTutorialGameMode::StartPlay()
{
    Super::StartPlay();
    SpawnNewWall(FName("TutorialWall_1"));
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
    HandleCurrentPhase();
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
    if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
    {
        Manager->StartGaugeObjective(EGaugeInteractionType::Hybrid, 100.f, 5.f, 30.f);
    }
}

void AADTutorialGameMode::HandlePhase_Radar()
{
    SpawnNewWall(FName("TutorialWall_2"));

    if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
    {
        Manager->StartGaugeObjective(EGaugeInteractionType::Tap, 100.f, 10.f, 0.f);
    }
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
    if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
    {
        Manager->StartGaugeObjective(EGaugeInteractionType::Tap, 100.f, 10.f, 0.f);
    }
}

void AADTutorialGameMode::HandlePhase_Drone()
{
    SpawnNewWall(FName("TutorialWall_7"));
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

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn) return;

    for (TActorIterator<ALight> It(GetWorld()); It; ++It)
    {
        ALight* LightActor = *It;
        if (LightActor)
        {
            if (LightActor->GetOwner() == PlayerPawn)
            {
                continue;
            }

            ULightComponent* LightComponent = LightActor->GetLightComponent();
            if (LightComponent && LightComponent->IsVisible())
            {
                DisabledLights.Add(LightActor);
                LightComponent->SetVisibility(false);
            }
        }
    }
    if (!TutorialPPV)
    {
        TutorialPPV = GetWorld()->SpawnActor<APostProcessVolume>();
        TutorialPPV->bUnbound = true;               
        TutorialPPV->BlendWeight = 1.0f;

        auto& S = TutorialPPV->Settings;
        S.bOverride_AutoExposureMinBrightness = true;
        S.bOverride_AutoExposureMaxBrightness = true;
        S.AutoExposureMinBrightness = 1.0f;         
        S.AutoExposureMaxBrightness = 1.0f;

        S.bOverride_AutoExposureSpeedUp = true;
        S.bOverride_AutoExposureSpeedDown = true;
        S.AutoExposureSpeedUp = 100.f;             
        S.AutoExposureSpeedDown = 100.f;
    }

    if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
    {
        Manager->StartGaugeObjective(EGaugeInteractionType::Tap, 100.f, 10.f, 0.f);
    }
}

void AADTutorialGameMode::HandlePhase_Items()
{
    SpawnNewWall(FName("TutorialWall_9"));
    ItemsPhaseProgress = 0;
}

void AADTutorialGameMode::HandlePhase_Battery()
{
    if (ATutorialManager* Manager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
    {
        const float TargetChargeAmount = 20.f;
        Manager->StartGaugeObjective(EGaugeInteractionType::Tap, TargetChargeAmount, 0.f, 0.f);
    }
}

void AADTutorialGameMode::HandlePhase_Drop()
{
}

void AADTutorialGameMode::HandlePhase_OxygenWarning()
{
    SpawnNewWall(FName("TutorialWall_12"));
}

void AADTutorialGameMode::HandlePhase_Revive()
{
    SpawnNewWall(FName("TutorialWall_13"));
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

void AADTutorialGameMode::DestroyActiveWall()
{
    if (IsValid(ActiveCurrentWall))
    {
        ActiveCurrentWall->SetActorEnableCollision(false);
        if (UNiagaraComponent* NiagaraComp = ActiveCurrentWall->FindComponentByClass<UNiagaraComponent>())
        {
            NiagaraComp->Deactivate();
        }
        ActiveCurrentWall->SetLifeSpan(2.0f);
        ActiveCurrentWall = nullptr;
    }
}

void AADTutorialGameMode::SpawnNewWall(FName WallTag)
{
    DestroyActiveWall();

    TArray<AActor*> FoundMarkers;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), WallTag, FoundMarkers);
    if (FoundMarkers.Num() > 0 && IsValid(CurrentWallClass))
    {
        AActor* Marker = FoundMarkers[0];
        FTransform SpawnTransform = Marker->GetActorTransform();

        FRotator SpawnRotation = SpawnTransform.GetRotation().Rotator();
        SpawnRotation.Yaw += 180.0f;
        SpawnTransform.SetRotation(SpawnRotation.Quaternion());
        ActiveCurrentWall = GetWorld()->SpawnActor<AActor>(CurrentWallClass, SpawnTransform);

        if (IsValid(ActiveCurrentWall))
        {
            ActiveCurrentWall->SetActorHiddenInGame(false);
            ActiveCurrentWall->SetActorEnableCollision(true);

            if (UNiagaraComponent* NiagaraComp = ActiveCurrentWall->FindComponentByClass<UNiagaraComponent>())
            {
                NiagaraComp->Activate(true);
            }
            UE_LOG(LogTemp, Warning, TEXT("[Wall] Spawned: %s"), *ActiveCurrentWall->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Wall] SpawnActor failed"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Wall] No marker or class invalid"));
    }
}
