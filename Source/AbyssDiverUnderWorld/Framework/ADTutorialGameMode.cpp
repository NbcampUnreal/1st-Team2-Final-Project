#include "Framework/ADTutorialGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Tutorial/TutorialManager.h"
#include "Framework/ADTutorialGameState.h"
#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"
#include "Interactable/OtherActors/TargetIndicators/TargetIndicatorManager.h"
#include "Framework/ADTutorialPlayerController.h"
#include "Engine/Light.h"
#include "EngineUtils.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Components/LightComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Interactable/OtherActors/Radars/RadarReturn2DComponent.h"
#include "EngineUtils.h"
#include "Animation/AnimMontage.h"
#include "Interactable/OtherActors/ADDroneSeller.h"
#include "Interactable/OtherActors/ADDrone.h"
#include "Engine/PostProcessVolume.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Engine/TargetPoint.h"
#include "Components/CapsuleComponent.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Character/PlayerComponent/OxygenComponent.h"
#include "Character/PlayerComponent/RagdollReplicationComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h" // (NPC->GetCharacterMovement() 쓸 때 필요)
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Subsystems/SoundSubsystem.h"
#include "GameFramework/PlayerState.h"

AADTutorialGameMode::AADTutorialGameMode()
{
	bBatteryGaugeStarted = false;
	ResurrectionDroneSpawnTag = FName("TutorialDroneSpawn");
	Tutorial_ActiveSeller = nullptr;
	Tutorial_ActiveDrone = nullptr;
	TutorialPPV = nullptr;
	bIsBodySubmittedInResurrectionPhase = false;
	bIsTypingFinishedForCurrentPhase = false;
	FixedRespawnPointTag = FName("RespawnPoint");
}

void AADTutorialGameMode::StartPlay()
{
	Super::StartPlay();

	if (AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		PlayerCharacter->SetMovementBlockedByTutorial(true);
	}

	AUnderwaterCharacter* PlayerChar = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (PlayerChar)
	{
		UOxygenComponent* OxygenComp = PlayerChar->FindComponentByClass<UOxygenComponent>();
		if (OxygenComp)
		{
			OxygenComp->SetTutorialMode(true);
		}
	}
	SpawnNewWall(FName("TutorialWall_1"));

	if (MuteDroneSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(GetWorld(), MuteDroneSoundMix);
	}

	TutorialPlayerController = Cast<AADPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	TArray<FName> TagsToHide = { FName("TutorialRock_D2"), FName("TutorialMonster_D2"), FName("TutorialFriendly_D2") };
	for (const FName& Tag : TagsToHide)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, FoundActors);
		for (AActor* Actor : FoundActors)
		{
			if (IsValid(Actor))
			{
				Actor->SetActorHiddenInGame(true);
				if (URadarReturn2DComponent* RadarComp = Actor->FindComponentByClass<URadarReturn2DComponent>())
				{
					RadarComp->SetAlwaysIgnore(true);
				}
			}
		}
	}

	if (StartPhaseOverride != ETutorialPhase::None)
	{
		if (AADTutorialGameState* TutorialGS = GetGameState<AADTutorialGameState>())
		{
			TutorialGS->SetCurrentPhase(StartPhaseOverride);
			HandleCurrentPhase();
		}
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			TutorialStartTimerHandle,
			this,
			&AADTutorialGameMode::StartFirstTutorialPhase,
			2.0f,
			false
		);
	}
}

void AADTutorialGameMode::StartFirstTutorialPhase()
{
	AdvanceTutorialPhase();
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
		switch (TutorialGS->GetCurrentPhase())
		{
		case ETutorialPhase::Step1_Movement:      HandlePhase_Movement(); break;
		case ETutorialPhase::Step2_Sprint:        HandlePhase_Sprint(); break;
		case ETutorialPhase::Step3_Oxygen:        HandlePhase_Oxygen(); break;
		case ETutorialPhase::Step4_Radar:         HandlePhase_Radar(); break;
		case ETutorialPhase::Dialogue_02:         HandlePhase_Dialogue_02(); break;
		case ETutorialPhase::Step5_Looting:       HandlePhase_Looting(); break;
		case ETutorialPhase::Step6_Inventory:     HandlePhase_Inventory(); break;
		case ETutorialPhase::Step7_Drone:         HandlePhase_Drone(); break;
		case ETutorialPhase::Dialogue_03:         break;
		case ETutorialPhase::Dialogue_LightOut:   HandlePhase_Dialogue_LightOut(); break;
		case ETutorialPhase::Step8_LightToggle:   HandlePhase_LightToggle(); break;
		case ETutorialPhase::Step9_Items:         HandlePhase_Items(); break;
		case ETutorialPhase::Dialogue_06:         HandlePhase_Dialogue_06(); break;
		case ETutorialPhase::Step10_Battery:      HandlePhase_Battery(); break;
		case ETutorialPhase::Step11_Drop:         HandlePhase_Drop(); break;
		case ETutorialPhase::Dialogue_05:         break;
		case ETutorialPhase::Step12_OxygenWarning: HandlePhase_OxygenWarning(); break;
		case ETutorialPhase::Dialogue_08:		  break;
		case ETutorialPhase::Step13_Revive:       HandlePhase_Revive(); break;
		case ETutorialPhase::Dialogue_04:         break;
		case ETutorialPhase::Step14_Die:          HandlePhase_Die(); break;
		case ETutorialPhase::Step15_Resurrection: HandlePhase_Resurrection(); break;
		case ETutorialPhase::Dialogue_07:         break;
		case ETutorialPhase::Complete:            HandlePhase_Complete(); break;
		default: break;
		}
	}
}

void AADTutorialGameMode::OnTutorialNPCStateChanged(AUnderwaterCharacter* Character, ECharacterState OldState, ECharacterState NewState)
{
	if (!Character || !TutorialNPC.IsValid() || Character != TutorialNPC.Get())
	{
		return;
	}

	if (OldState == ECharacterState::Groggy && NewState == ECharacterState::Normal)
	{
		TutorialNPC->RequestPlayEmote(1);

		GetWorldTimerManager().ClearTimer(EmoteToNextTimerHandle);
		GetWorldTimerManager().SetTimer(
			EmoteToNextTimerHandle,
			this,
			&AADTutorialGameMode::AdvanceTutorialPhase,
			EmoteToNextPhaseDelay,
			false
		);
	}
}

void AADTutorialGameMode::HandlePhase_Movement()
{
	if (AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		PlayerCharacter->SetMovementBlockedByTutorial(false);
	}
}
void AADTutorialGameMode::HandlePhase_Oxygen() {}
void AADTutorialGameMode::HandlePhase_Dialogue_05() {}

void AADTutorialGameMode::HandlePhase_Complete()
{
	if (IsValid(ExitAlarmSound))
	{
		float VolumeToPlay = 1.0f;
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (USoundSubsystem* TutorialSoundSubsystem = GameInstance->GetSubsystem<USoundSubsystem>())
			{
				VolumeToPlay = TutorialSoundSubsystem->GetSFXVolume() * TutorialSoundSubsystem->GetMasterVolume();
			}
		}
		LoopingPhaseSoundComponent = UGameplayStatics::SpawnSound2D(GetWorld(), ExitAlarmSound, VolumeToPlay);
	}

	TArray<AActor*> FoundLadders;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialExitLadder"), FoundLadders);

	if (FoundLadders.Num() > 0 && IsValid(IndicatingTargetClass))
	{
		AActor* LadderActor = FoundLadders[0];

		if (AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, LadderActor->GetActorTransform()))
		{
			Indicator->SetupIndicator(LadderActor, LadderExitIndicatorIcon);

			if (ATargetIndicatorManager* TargetMgr = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
			{
				TargetMgr->RegisterNewTarget(Indicator);
			}
			TrackPhaseActor(Indicator);
		}
	}
}

void AADTutorialGameMode::HandlePhase_Sprint()
{
	if (ATutorialManager* Mgr = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Mgr->StartGaugeObjective(EGaugeInteractionType::Hybrid, 100.f, 5.f, 30.f);
	}
}

void AADTutorialGameMode::HandlePhase_Radar()
{
	SpawnNewWall(FName("TutorialWall_2"));

	if (ATutorialManager* Mgr = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Mgr->StartGaugeObjective(EGaugeInteractionType::Tap, 100.f, 100.f, 0.f);
	}

	TArray<FName> TagsToShow = { FName("TutorialRock_D2"), FName("TutorialMonster_D2"), FName("TutorialFriendly_D2") };
	for (const FName& Tag : TagsToShow)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, FoundActors);
		for (AActor* Actor : FoundActors)
		{
			if (IsValid(Actor))
			{
				Actor->SetActorHiddenInGame(false);
				if (URadarReturn2DComponent* RadarComp = Actor->FindComponentByClass<URadarReturn2DComponent>())
				{
					RadarComp->SetAlwaysIgnore(false);
				}
			}
		}
	}
}

void AADTutorialGameMode::HandlePhase_Dialogue_02()
{
	if (IsValid(RadarAlarmSound))
	{
		float VolumeToPlay = 1.0f;
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (USoundSubsystem* TutorialSoundSubsystem = GameInstance->GetSubsystem<USoundSubsystem>())
			{
				VolumeToPlay = TutorialSoundSubsystem->GetSFXVolume() * TutorialSoundSubsystem->GetMasterVolume();
			}
		}
		LoopingPhaseSoundComponent = UGameplayStatics::SpawnSound2D(GetWorld(), RadarAlarmSound, VolumeToPlay);
	}

	if (!IndicatingTargetClass) return;

	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), DialogueTargetSpawnTag, SpawnPoints);
	if (SpawnPoints.Num() == 0) return;

	AActor* SpawnPoint = SpawnPoints[0];
	if (AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, SpawnPoint->GetActorTransform()))
	{
		Indicator->SetupIndicator(nullptr, DialogueIndicatorIcon);
		if (ATargetIndicatorManager* TargetMgr = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
		{
			TargetMgr->RegisterNewTarget(Indicator);
		}
		TrackPhaseActor(Indicator);
	}
}

void AADTutorialGameMode::HandlePhase_Looting()
{
	TArray<FName> TagsToDestroy = { FName("TutorialMonster_D2"), FName("TutorialFriendly_D2") };
	for (const FName& Tag : TagsToDestroy)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, FoundActors);
		for (AActor* Actor : FoundActors)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy();
			}
		}
	}
	if (!LootableOreClass || !IndicatingTargetClass) return;

	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), OreSpawnTag, SpawnPoints);
	if (SpawnPoints.Num() == 0) return;

	AActor* SpawnPoint = SpawnPoints[0];
	const FTransform SpawnTM = SpawnPoint->GetActorTransform();

	if (AActor* SpawnedOre = GetWorld()->SpawnActor<AActor>(LootableOreClass, SpawnTM))
	{
		TrackPhaseActor(SpawnedOre);

		if (AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, SpawnTM))
		{
			Indicator->SetupIndicator(SpawnedOre, LootingOreIcon);
			if (ATargetIndicatorManager* TargetMgr = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
			{
				TargetMgr->RegisterNewTarget(Indicator);
			}
			BindIndicatorToOwner(SpawnedOre, Indicator);
		}
	}
}

void AADTutorialGameMode::HandlePhase_Inventory()
{
	if (ATutorialManager* Mgr = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Mgr->StartGaugeObjective(EGaugeInteractionType::Hold, 100.f, 5.f, 100.f);
	}
}

void AADTutorialGameMode::HandlePhase_Drone()
{
	SpawnNewWall(FName("TutorialWall_7"));

	TArray<AActor*> FoundDrones;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("stepdrone"), FoundDrones);
	if (FoundDrones.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandlePhase_Drone: 'stepdrone' 태그를 가진 드론을 찾을 수 없습니다."));
		return;
	}

	AADDrone* Drone = Cast<AADDrone>(FoundDrones[0]);

	TArray<AActor*> FoundSellers;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("stepdroneseller"), FoundSellers);
	if (FoundSellers.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandlePhase_Drone: 'stepdroneseller' 태그를 가진 셀러를 찾을 수 없습니다."));
		return;
	}

	AADDroneSeller* Seller = Cast<AADDroneSeller>(FoundSellers[0]);

	if (!Drone || !Seller)
	{
		UE_LOG(LogTemp, Error, TEXT("HandlePhase_Drone: 드론 또는 셀러를 올바른 클래스로 변환할 수 없습니다."));
		return;
	}

	Drone->CurrentSeller = Seller;
	Seller->SetCurrentDrone(Drone);

	Drone->SetActorHiddenInGame(false);
	Seller->SetActorHiddenInGame(false);
	Seller->Activate();
	Drone->Activate();
	Drone->M_PlayTutorialAlarmSound();

	if (UADInteractableComponent* SellerInteractComp = Seller->GetInteractableComponent())
	{
		SellerInteractComp->SetInteractable(true);
	}
	if (UADInteractableComponent* DroneInteractComp = Drone->GetInteractableComponent())
	{
		DroneInteractComp->SetInteractable(true);
	}

	if (IndicatingTargetClass && IsValid(Seller))
	{
		if (AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, Seller->GetActorTransform()))
		{
			Indicator->SetupIndicator(Seller, DroneIndicatorIcon);

			if (ATargetIndicatorManager* TargetMgr = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
			{
				TargetMgr->RegisterNewTarget(Indicator);
			}
			TrackPhaseActor(Indicator);
		}
	}
}

void AADTutorialGameMode::HandlePhase_Dialogue_LightOut()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USoundSubsystem* TutorialSoundSubsystem = GameInstance->GetSubsystem<USoundSubsystem>())
		{
			if (IsValid(LightOutSound))
			{
				const float NewVolume = TutorialSoundSubsystem->GetSFXVolume() * TutorialSoundSubsystem->GetMasterVolume();
				UGameplayStatics::PlaySound2D(GetWorld(), LightOutSound, NewVolume);
			}
		}
	}
	else if (IsValid(LightOutSound))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), LightOutSound);
	}

	DisabledLights.Empty();
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		for (TActorIterator<ALight> It(GetWorld()); It; ++It)
		{
			if (ALight* LightActor = *It)
			{
				if (LightActor->GetOwner() != PlayerPawn)
				{
					if (ULightComponent* Lc = LightActor->GetLightComponent())
					{
						if (Lc->IsVisible())
						{
							DisabledLights.Add(LightActor);
							Lc->SetVisibility(false);
						}
					}
				}
			}
		}
	}
	if (!TutorialPPV)
	{
		TutorialPPV = GetWorld()->SpawnActor<APostProcessVolume>();
		if (TutorialPPV)
		{
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
	}
	FTimerHandle DelayTimer;
	GetWorldTimerManager().SetTimer(DelayTimer, this, &AADTutorialGameMode::AdvanceTutorialPhase, 2.0f, false);
}

void AADTutorialGameMode::HandlePhase_LightToggle()
{
	if (ATutorialManager* Mgr = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Mgr->StartGaugeObjective(EGaugeInteractionType::Tap, 100.f, 100.f, 0.f);
	}
}

void AADTutorialGameMode::HandlePhase_Items()
{
	SpawnNewWall(FName("TutorialWall_9"));
}

void AADTutorialGameMode::HandlePhase_Dialogue_06()
{
	OnPhaseBatteryStart();
	ItemsPhaseProgress = 0;
}

void AADTutorialGameMode::HandlePhase_Battery()
{
	if (bBatteryGaugeStarted) return;
	bBatteryGaugeStarted = true;

	float StartBattPct = BatteryStartPercentOverride < 0.f ? 50.f : BatteryStartPercentOverride;
	StartBattPct = FMath::Clamp(StartBattPct, 0.f, 100.f);
	const float TargetChargeAmount = FMath::Max(0.f, 100.f - StartBattPct);

	if (TargetChargeAmount <= KINDA_SMALL_NUMBER)
	{
		AdvanceTutorialPhase();
		return;
	}

	if (ATutorialManager* Mgr = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Mgr->StartGaugeObjective(EGaugeInteractionType::Tap, TargetChargeAmount, 0.f, 0.f);
	}
}

void AADTutorialGameMode::HandlePhase_Drop()
{
	if (ATutorialManager* Mgr = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Mgr->StartGaugeObjective(EGaugeInteractionType::Tap, 100.f, 50.f, 0.f);
	}
}

void AADTutorialGameMode::HandlePhase_OxygenWarning()
{
	SpawnNewWall(FName("TutorialWall_12"));
}

void AADTutorialGameMode::HandlePhase_Revive()
{
	if (IsValid(ReviveNPCIndicatorSound))
	{
		float VolumeToPlay = 1.0f;
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (USoundSubsystem* TutorialSoundSubsystem = GameInstance->GetSubsystem<USoundSubsystem>())
			{
				VolumeToPlay = TutorialSoundSubsystem->GetSFXVolume() * TutorialSoundSubsystem->GetMasterVolume();
			}
		}
		LoopingPhaseSoundComponent = UGameplayStatics::SpawnSound2D(GetWorld(), ReviveNPCIndicatorSound, VolumeToPlay);
	}

	SpawnNewWall(FName("TutorialWall_13"));

	if (!GroggyNPCClass) return;

	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), GroggyNPCSpawnTag, SpawnPoints);
	if (SpawnPoints.Num() == 0) return;

	const FTransform SpawnTM = SpawnPoints[0]->GetActorTransform();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;

	if (TutorialNPC.IsValid())
	{
		TutorialNPC->OnCharacterStateChangedDelegate.RemoveDynamic(this, &AADTutorialGameMode::OnTutorialNPCStateChanged);
		TutorialNPC->Destroy();
		TutorialNPC.Reset();
	}

	if (AUnderwaterCharacter* SpawnedNPC = GetWorld()->SpawnActor<AUnderwaterCharacter>(GroggyNPCClass, SpawnTM, Params))
	{
		TutorialNPC = SpawnedNPC;
		TutorialNPC->OnCharacterStateChangedDelegate.AddDynamic(this, &AADTutorialGameMode::OnTutorialNPCStateChanged);
		SpawnedNPC->SetCharacterState(ECharacterState::Groggy);

		if (IndicatingTargetClass)
		{
			if (AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, SpawnedNPC->GetActorTransform()))
			{
				Indicator->SetupIndicator(SpawnedNPC, NPCIcon);
				if (ATargetIndicatorManager* TargetMgr = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
				{
					TargetMgr->RegisterNewTarget(Indicator);
				}
				TrackPhaseActor(Indicator);
			}
		}
	}
}

void AADTutorialGameMode::HandlePhase_Die()
{
	// 즉사 연출: 몽타주 중단 후 약간의 지연을 두고 Death 상태로
	if (!TutorialNPC.IsValid()) return;

	TutorialNPC->RequestStopAllMontage(EPlayAnimationTarget::BothPersonMesh, 0.1f);

	FTimerHandle KillTimer;
	GetWorldTimerManager().SetTimer(
		KillTimer,
		[this]()
		{
			if (TutorialNPC.IsValid())
			{
				TutorialNPC->SetCharacterState(ECharacterState::Death);
			}
		},
		0.15f,
		false
	);
	if (IndicatingTargetClass)
	{
		if (AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, TutorialNPC->GetActorTransform()))
		{
			Indicator->SetupIndicator(TutorialNPC.Get(), NPCIcon);
			if (ATargetIndicatorManager* TargetMgr = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
			{
				TargetMgr->RegisterNewTarget(Indicator);
			}
			TrackPhaseActor(Indicator);
		}
	}
	if (ATutorialManager* Mgr = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
	{
		Mgr->StartGaugeObjective(EGaugeInteractionType::Tap, 100.f, 50.f, 0.f);
	}
}

void AADTutorialGameMode::HandlePhase_Resurrection()
{
	// 셀러/드론을 레벨에서 찾아 유효성 검증 후 상호참조와 인터랙션 상태 세팅
	if (!HasAuthority()) return;

	bIsBodySubmittedInResurrectionPhase = false;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialSeller"), FoundActors);
	Tutorial_ActiveSeller = FoundActors.Num() > 0 ? Cast<AADDroneSeller>(FoundActors[0]) : nullptr;

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TutorialDrone"), FoundActors);
	Tutorial_ActiveDrone = FoundActors.Num() > 0 ? Cast<AADDrone>(FoundActors[0]) : nullptr;

	if (!IsValid(Tutorial_ActiveSeller) || !IsValid(Tutorial_ActiveDrone))
	{
		UE_LOG(LogTemp, Error, TEXT("Resurrection Phase: 레벨에서 TutorialSeller 또는 TutorialDrone을 찾을 수 없습니다."));
		return;
	}

	Tutorial_ActiveSeller->SetCurrentDrone(Tutorial_ActiveDrone);
	Tutorial_ActiveDrone->CurrentSeller = Tutorial_ActiveSeller;

	Tutorial_ActiveSeller->Activate();
	if (UADInteractableComponent* SellerInteractComp = Tutorial_ActiveSeller->GetInteractableComponent())
	{
		SellerInteractComp->SetInteractable(true);
	}

	Tutorial_ActiveDrone->SetActorHiddenInGame(false);
	Tutorial_ActiveSeller->SetActorHiddenInGame(false);
	Tutorial_ActiveDrone->SetActorEnableCollision(true);
	Tutorial_ActiveDrone->Activate();
	Tutorial_ActiveDrone->M_PlayTutorialAlarmSound();

	if (UADInteractableComponent* DroneInteractComp = Tutorial_ActiveDrone->GetInteractableComponent())
	{
		DroneInteractComp->SetInteractable(false);
	}

	if (IndicatingTargetClass && IsValid(Tutorial_ActiveSeller))
	{
		if (AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, Tutorial_ActiveSeller->GetActorTransform()))
		{

			Indicator->SetupIndicator(Tutorial_ActiveSeller, DroneIndicatorIcon);

			if (ATargetIndicatorManager* TargetMgr = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
			{
				TargetMgr->RegisterNewTarget(Indicator);
			}

			TrackPhaseActor(Indicator);
		}
	}
}

void AADTutorialGameMode::TriggerResurrectionSequence()
{
	// 시체 제출 완료 플래그 확인 후 부활 시퀀스 시작
	UE_LOG(LogTemp, Warning, TEXT("튜토리얼 TriggerResurrectionSequence - 함수 진입 성공!"));

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("튜토리얼  TriggerResurrectionSequence - 서버가 아니므로 로직 중단."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("튜토리얼 TriggerResurrectionSequence - 서버 권한 확인 완료."));

	UE_LOG(LogTemp, Warning, TEXT("튜토리얼  TriggerResurrectionSequence - 'bIsBodySubmittedInResurrectionPhase' 상태: %s"),
		bIsBodySubmittedInResurrectionPhase ? TEXT("True") : TEXT("False"));

	if (!bIsBodySubmittedInResurrectionPhase)
	{
		UE_LOG(LogTemp, Error, TEXT("튜토리얼 TriggerResurrectionSequence - 시체가 제출되지 않은 상태이므로 로직 중단."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("튜토리얼 TriggerResurrectionSequence - 모든 조건 통과! 지금부터 부활 로직을 시작합니다."));

	if (IsValid(Tutorial_ActiveDrone))
	{
		Tutorial_ActiveDrone->StartRising();
	}

	ReviveTutorialNPCAtDrone(Tutorial_ActiveDrone);
}

void AADTutorialGameMode::NotifyBodySubmitted_Implementation(AUnderwaterCharacter* SubmittingPlayer)
{
	if (!HasAuthority() || !SubmittingPlayer || !TutorialNPC.IsValid()) return;

	AUnderwaterCharacter* Corpse = TutorialNPC.Get();
	if (!SubmittingPlayer->GetBoundCharacters().Contains(Corpse)) return;

	SubmittingPlayer->UnbindAllBoundCharacters();

	if (Corpse)
	{
		Corpse->Destroy();
	}

	bIsBodySubmittedInResurrectionPhase = true;

	if (auto* C = Tutorial_ActiveSeller ? Tutorial_ActiveSeller->GetInteractableComponent() : nullptr)
		C->SetInteractable(false);
	if (auto* C = Tutorial_ActiveDrone ? Tutorial_ActiveDrone->GetInteractableComponent() : nullptr)
		C->SetInteractable(true);
}

AUnderwaterCharacter* AADTutorialGameMode::GetTutorialNPC() const
{
	return TutorialNPC.Get();
}

bool AADTutorialGameMode::IsTypingFinishedForCurrentPhase() const
{
	return bIsTypingFinishedForCurrentPhase;
}

void AADTutorialGameMode::OnTypingAnimationFinished()
{
	bIsTypingFinishedForCurrentPhase = true;
}

void AADTutorialGameMode::HidePhaseActors()
{
	if (IsValid(LoopingPhaseSoundComponent))
	{
		LoopingPhaseSoundComponent->Stop();
		LoopingPhaseSoundComponent = nullptr;
	}

	for (int32 i = ActorsToShowThisPhase.Num() - 1; i >= 0; --i)
	{
		if (AActor* ActorToDestroy = ActorsToShowThisPhase[i])
		{
			if (TutorialNPC.IsValid() && ActorToDestroy == TutorialNPC.Get())
			{
				continue;
			}
			ActorToDestroy->OnDestroyed.RemoveAll(this);
			ActorToDestroy->Destroy();
		}
	}
	ActorsToShowThisPhase.Empty();
}

void AADTutorialGameMode::OnPlayerItemAction(EPlayerActionTrigger ItemActionType)
{
	// 특정 페이즈에서만 입력을 카운팅/통과 트리거로 사용
	if (AADTutorialGameState* TutorialGS = GetGameState<AADTutorialGameState>())
	{
		const ETutorialPhase CurrentPhase = TutorialGS->GetCurrentPhase();

		if (CurrentPhase == ETutorialPhase::Dialogue_06)
		{
			if (ItemsPhaseProgress == 0 && ItemActionType == EPlayerActionTrigger::UseItem1) { ItemsPhaseProgress++; }
			else if (ItemsPhaseProgress == 1 && ItemActionType == EPlayerActionTrigger::UseItem2) { ItemsPhaseProgress++; }
			else if (ItemsPhaseProgress == 2 && ItemActionType == EPlayerActionTrigger::UseItem3) { AdvanceTutorialPhase(); }
		}
		else if (CurrentPhase == ETutorialPhase::Step11_Drop)
		{
			if (ItemActionType == EPlayerActionTrigger::Drop)
			{
				if (ATutorialManager* Mgr = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
				{
					Mgr->NotifyInteractionStart();
				}
			}
		}
		else if (CurrentPhase == ETutorialPhase::Step14_Die)
		{
			if (ItemActionType == EPlayerActionTrigger::Interact)
			{
				if (ATutorialManager* Mgr = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass())))
				{
					Mgr->NotifyInteractionStart();
				}
			}
		}
	}
}

void AADTutorialGameMode::TrackPhaseActor(AActor* Actor)
{
	if (IsValid(Actor))
	{
		ActorsToShowThisPhase.Add(Actor);
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
		FTransform SpawnTM = Marker->GetActorTransform();
		FRotator SpawnRot = SpawnTM.GetRotation().Rotator();
		SpawnRot.Yaw += 180.0f;
		SpawnTM.SetRotation(SpawnRot.Quaternion());

		ActiveCurrentWall = GetWorld()->SpawnActor<AActor>(CurrentWallClass, SpawnTM);
		if (IsValid(ActiveCurrentWall))
		{
			ActiveCurrentWall->SetActorHiddenInGame(false);
			ActiveCurrentWall->SetActorEnableCollision(true);
			if (UNiagaraComponent* NiagaraComp = ActiveCurrentWall->FindComponentByClass<UNiagaraComponent>())
			{
				NiagaraComp->Activate(true);
			}
		}
	}
}

void AADTutorialGameMode::SpawnTutorialDronePair(TObjectPtr<AADDroneSeller>& OutSeller, TObjectPtr<AADDrone>& OutDrone)
{
	// 드론/셀러 페어 스폰(실패 시 안전 정리)
	OutSeller = nullptr;
	OutDrone = nullptr;

	if (!ensure(TutorialDroneSellerClass) || !ensure(TutorialDroneClass))
	{
		return;
	}

	const FTransform SpawnTM = GetResurrectionSpawnTransform();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;

	OutSeller = GetWorld()->SpawnActor<AADDroneSeller>(TutorialDroneSellerClass, SpawnTM, Params);
	if (!IsValid(OutSeller))
	{
		return;
	}

	FTransform DroneTM = SpawnTM;
	FVector Loc = DroneTM.GetLocation();
	Loc.Z += 200.f;
	DroneTM.SetLocation(Loc);

	OutDrone = GetWorld()->SpawnActor<AADDrone>(TutorialDroneClass, DroneTM, Params);
	if (!IsValid(OutDrone))
	{
		OutSeller->Destroy();
		OutSeller = nullptr;
		return;
	}

	OutSeller->Activate();
	OutSeller->SetCurrentDrone(OutDrone);
	OutDrone->CurrentSeller = OutSeller;
}

FTransform AADTutorialGameMode::GetResurrectionSpawnTransform() const
{
	// 지정 태그 우선, 없으면 플레이어 전방 300cm
	TArray<AActor*> Points;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), ResurrectionDroneSpawnTag, Points);
	if (Points.Num() > 0 && IsValid(Points[0]))
	{
		return Points[0]->GetActorTransform();
	}

	if (APawn* P = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		const FVector Fwd = P->GetActorForwardVector();
		return FTransform(P->GetActorRotation(), P->GetActorLocation() + Fwd * 300.f, FVector(1.f));
	}

	return FTransform::Identity;
}

FTransform AADTutorialGameMode::ChooseTutorialRespawnTransform(const AADDrone* Drone) const
{
	if (!IsValid(Drone))
	{
		UE_LOG(LogTemp, Error, TEXT("튜토리얼 ChooseRespawnTransform - 드론이 유효하지 않습니다!"));
		return FTransform::Identity;
	}

	const TArray<ATargetPoint*>& Points = Drone->GetPlayerRespawnLocations();
	UE_LOG(LogTemp, Warning, TEXT("튜토리얼 ChooseRespawnTransform - 드론에서 찾은 TargetPoint 개수: %d"), Points.Num());

	for (int32 i = 0; i < Points.Num(); ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("배열 [%d]: %s"), i, *GetNameSafe(Points[i]));
	}

	if (Points.Num() > 0)
	{
		const int32 Idx = FMath::RandRange(0, Points.Num() - 1);
		if (IsValid(Points[Idx]))
		{
			UE_LOG(LogTemp, Log, TEXT("튜토리얼 ChooseRespawnTransform - TargetPoint '%s' 선택"),
				*GetNameSafe(Points[Idx]));
			return Points[Idx]->GetActorTransform();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("튜토리얼 ChooseRespawnTransform - 유효한 TargetPoint 없음 → 랜덤 폴백"));

	const FVector Center = Drone->GetActorLocation();
	const float R = Drone->GetReviveDistance();
	const FVector RandDir = FMath::VRand();
	const FVector Dir2D = FVector(RandDir.X, RandDir.Y, 0.f).GetSafeNormal();
	const float Radius = FMath::FRandRange(R * 0.3f, R);
	FVector Loc = Center + Dir2D * Radius;
	Loc.Z = Center.Z;

	return FTransform(Drone->GetActorRotation(), Loc, FVector(1.f));
}

bool AADTutorialGameMode::TutorialTryGetRandomTaggedTargetPointTransform(const FName& Tag, FTransform& OutTM) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	TArray<AActor*> Points;
	// TargetPoint + Tag 동시 필터
	UGameplayStatics::GetAllActorsOfClassWithTag(World, ATargetPoint::StaticClass(), Tag, Points);

	if (Points.Num() <= 0) return false;

	const int32 Idx = FMath::RandRange(0, Points.Num() - 1);
	if (AActor* P = Points[Idx])
	{
		OutTM = P->GetActorTransform();
		return true;
	}
	return false;
}

FTransform AADTutorialGameMode::GetBestTutorialRespawnTransform(const AADDrone* Drone) const
{
	FTransform TM;

	if (TutorialTryGetRandomTaggedTargetPointTransform(FixedRespawnPointTag, TM))
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Respawn] Use tagged TargetPoint '%s'."), *FixedRespawnPointTag.ToString());
		return TM;
	}

	return ChooseTutorialRespawnTransform(Drone);
}

void AADTutorialGameMode::ReviveTutorialNPCAtDrone(AADDrone* Drone)
{
	if (!HasAuthority() || !IsValid(Drone)) return;

	const FTransform RespawnTM = GetBestTutorialRespawnTransform(Drone);

	if (TutorialNPC.IsValid())
	{
		TutorialNPC->OnCharacterStateChangedDelegate.RemoveDynamic(this, &AADTutorialGameMode::OnTutorialNPCStateChanged);
		TutorialNPC->Destroy();
		TutorialNPC.Reset();
	}

	if (!GroggyNPCClass) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (AUnderwaterCharacter* NewNPC = GetWorld()->SpawnActor<AUnderwaterCharacter>(GroggyNPCClass, RespawnTM, Params))
	{
		TutorialNPC = NewNPC;
		TutorialNPC->OnCharacterStateChangedDelegate.AddDynamic(this, &AADTutorialGameMode::OnTutorialNPCStateChanged);

		TutorialNPC->SetCharacterState(ECharacterState::Groggy);
		TWeakObjectPtr<AUnderwaterCharacter> WeakNPC = TutorialNPC;
		FTimerHandle ReviveTimer;
		GetWorldTimerManager().SetTimer(
			ReviveTimer,
			[WeakNPC]()
			{
				if (WeakNPC.IsValid())
				{
					WeakNPC->SetCharacterState(ECharacterState::Normal);
				}
			},
			0.1f,
			false
		);

		UE_LOG(LogTemp, Log, TEXT("NPC를 %s 위치에 스폰했습니다. 곧 Normal 상태로 전환하여 이모트를 재생합니다."), *RespawnTM.GetLocation().ToString());

		if (IndicatingTargetClass)
		{
			if (AIndicatingTarget* Indicator = GetWorld()->SpawnActor<AIndicatingTarget>(IndicatingTargetClass, NewNPC->GetActorTransform()))
			{
				Indicator->SetupIndicator(NewNPC, NPCIcon);
				if (ATargetIndicatorManager* TargetMgr = *TActorIterator<ATargetIndicatorManager>(GetWorld()))
				{
					TargetMgr->RegisterNewTarget(Indicator);
				}
				TrackPhaseActor(Indicator);
			}
		}
	}
}

ATargetPoint* AADTutorialGameMode::FindTargetPointByTag(const FName& Tag) const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	for (TActorIterator<ATargetPoint> It(World); It; ++It)
	{
		if (It->ActorHasTag(Tag))
		{
			return *It;
		}
	}
	return nullptr;
}

void AADTutorialGameMode::ReviveSinglePlayerAtDrone(int8 PlayerIndex, const AADDrone* Drone)
{
	if (!HasAuthority() || !IsValid(Drone)) return;

	const FTransform BestTM = GetBestTutorialRespawnTransform(Drone);
	RestartTutorialPlayerFromIndex(PlayerIndex, BestTM.GetLocation());
}

AADTutorialPlayerController* AADTutorialGameMode::FindTutorialPlayerControllerByIndex(int8 PlayerIndex) const
{
	for (AADTutorialPlayerController* PC : TActorRange<AADTutorialPlayerController>(GetWorld()))
	{
		if (const AADPlayerState* PS = Cast<AADPlayerState>(PC->PlayerState))
		{
			if (PS->GetPlayerIndex() == PlayerIndex)
			{
				return PC;
			}
		}
	}
	return nullptr;
}

void AADTutorialGameMode::RestartTutorialPlayerFromIndex(int8 PlayerIndex, const FVector& SpawnLocation)
{
	AADTutorialPlayerController* PC = FindTutorialPlayerControllerByIndex(PlayerIndex);
	if (!PC) return;

	if (APawn* Pawn = PC->GetPawn())
	{
		PC->UnPossess();
		Pawn->Destroy();
	}

	const FRotator SpawnRot = FRotator::ZeroRotator;
	RestartPlayerAtTransform(PC, FTransform(SpawnRot, SpawnLocation));
}

FVector AADTutorialGameMode::GetTutorialRandomLocation(const FVector& Center, float Distance) const
{
	float CapsuleRadius = 34.f, CapsuleHalfHeight = 88.f;
	if (AUnderwaterCharacter* DefaultChar = DefaultPawnClass ? DefaultPawnClass->GetDefaultObject<AUnderwaterCharacter>() : nullptr)
	{
		if (const UCapsuleComponent* Cap = DefaultChar->GetCapsuleComponent())
		{
			CapsuleRadius = Cap->GetScaledCapsuleRadius();
			CapsuleHalfHeight = Cap->GetScaledCapsuleHalfHeight();
		}
	}

	FVector Candidate = Center;
	for (int tries = 0; tries < 10; ++tries)
	{
		Candidate = Center + FMath::VRand() * Distance;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.bTraceComplex = true;

		const bool bHit = GetWorld()->SweepSingleByChannel(
			Hit, Candidate, Candidate, FQuat::Identity,
			ECC_Visibility, FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), Params);

		if (!bHit) return Candidate;
	}
	return Candidate;
}

void AADTutorialGameMode::RequestFinishTutorial()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 2.0f, FLinearColor::Black, false, true);

		FTimerHandle LevelChangeTimer;
		GetWorldTimerManager().SetTimer(LevelChangeTimer, [this]() {
			UGameplayStatics::OpenLevel(this, FName("MainLevel"));
			}, 2.0f, false);
	}
}