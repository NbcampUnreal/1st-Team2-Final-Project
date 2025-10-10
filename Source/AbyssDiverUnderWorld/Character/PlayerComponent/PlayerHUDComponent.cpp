#include "PlayerHUDComponent.h"

#include "AbyssDiverUnderWorld.h"

#include "Character/PlayerHUDWidget.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/PlayerComponent/OxygenComponent.h"
#include "Character/PlayerComponent/StaminaComponent.h"
#include "Character/StatComponent.h"

#include "Framework/ADInGameState.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADPlayerController.h"

#include "UI/ResultScreen.h"
#include "UI/ResultScreenSlot.h"
#include "UI/PlayerStatusWidget.h"
#include "UI/MissionsOnHUDWidget.h"
#include "UI/CrosshairWidget.h"
#include "UI/SpectatorHUDWidget.h"
#include "UI/RadarWidgets/Radar2DWidget.h"

#include "Interactable/OtherActors/ADDroneSeller.h"
#include "Subsystems/SoundSubsystem.h"

#include "EngineUtils.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory/ADInventoryComponent.h"
#include "Character/PlayerComponent/DepthComponent.h"
#include "Interactable/OtherActors/ADDrone.h"
#include "UI/DepthWidget.h"
#include "UI/InteractPopupWidget.h"
#include "Subsystems/ADWorldSubsystem.h"

UPlayerHUDComponent::UPlayerHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	ConstructorHelpers::FClassFinder<UInteractPopupWidget> PopupFinder(TEXT("/Game/_AbyssDiver/Blueprints/UI/InteractableUI/WBP_InteractPopupWidget"));
	if (PopupFinder.Succeeded())
	{
		PopupWidgetClass = PopupFinder.Class;
	}
}

void UPlayerHUDComponent::BindGameState()
{
	AADInGameState* GS = Cast<AADInGameState>(GetWorld()->GetGameState());
	if (GS == nullptr)
	{
		LOGV(Warning, TEXT("GS == nullptr"));
		return;
	}

	AADDroneSeller* CurrentDroneSeller = GS->GetCurrentDroneSeller();
	if (CurrentDroneSeller == nullptr)
	{
		LOGV(Warning, TEXT("CurrentDroneSeller == nullptr, Server? : %d"), GetOwner()->GetNetMode() != ENetMode::NM_Client);
		return;
	}

	PlayerStatusWidget->SetDroneTargetText(CurrentDroneSeller->GetTargetMoney());
	PlayerStatusWidget->SetDroneCurrentText(CurrentDroneSeller->GetCurrentMoney());
	PlayerStatusWidget->SetMoneyProgressBar(CurrentDroneSeller->GetMoneyRatio());

	CurrentDroneSeller->OnCurrentMoneyChangedDelegate.RemoveAll(PlayerStatusWidget);
	CurrentDroneSeller->OnCurrentMoneyChangedDelegate.AddUObject(PlayerStatusWidget, &UPlayerStatusWidget::SetDroneCurrentText);

	CurrentDroneSeller->OnTargetMoneyChangedDelegate.RemoveAll(PlayerStatusWidget);
	CurrentDroneSeller->OnTargetMoneyChangedDelegate.AddUObject(PlayerStatusWidget, &UPlayerStatusWidget::SetDroneTargetText);

	CurrentDroneSeller->OnMoneyRatioChangedDelegate.RemoveAll(PlayerStatusWidget);
	CurrentDroneSeller->OnMoneyRatioChangedDelegate.AddUObject(PlayerStatusWidget, &UPlayerStatusWidget::SetMoneyProgressBar);
}

void UPlayerHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	AADPlayerController* PlayerController = Cast<AADPlayerController>(GetOwner());
	AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(PlayerController->GetPawn());

	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	PlayerController->OnPossessedPawnChanged.AddDynamic(this, &UPlayerHUDComponent::OnPossessedPawnChanged);
	PlayerController->OnSpectateChanged.AddDynamic(this, &UPlayerHUDComponent::OnSpectatingStateChanged);
	
	// Test HUD 생성
	if (HudWidgetClass)
	{
		HudWidget = CreateWidget<UPlayerHUDWidget>(PlayerController, HudWidgetClass);
		if (HudWidget)
		{
			HudWidget->AddToViewport();
			HudWidget->BindWidget(PlayerController->GetPawn());
		}
	}
	SetTestHUDVisibility(false);

	// Crosshair Widget 생성
	if (CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UCrosshairWidget>(PlayerController, CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport();
			CrosshairWidget->BindWidget(PlayerController->GetPawn());
		}
	}

	// Player Status UI 생성
	if (PlayerStatusWidgetClass)
	{
		PlayerStatusWidget = CreateWidget<UPlayerStatusWidget>(PlayerController, PlayerStatusWidgetClass);
		if (PlayerStatusWidget)
		{
			PlayerStatusWidget->AddToViewport();
			PlayerStatusWidget->SetCompassObjectWidgetVisible(true);
		}
	}

	// Bind Character Components
	if (Character) 
	{
		// 스탯 컴포넌트
		if (UStatComponent* StatComponent = Character->FindComponentByClass<UStatComponent>())
		{
			StatComponent->OnHealthChanged.AddDynamic(this, &UPlayerHUDComponent::UpdateHealthHUD);
			UpdateHealthHUD(StatComponent->GetCurrentHealth(), StatComponent->GetMaxHealth());
		}

		// 산소
		if (UOxygenComponent* OxygenComp = Character->FindComponentByClass<UOxygenComponent>())
		{
			OxygenComp->OnOxygenLevelChanged.AddDynamic(this, &UPlayerHUDComponent::UpdateOxygenHUD);
			UpdateOxygenHUD(OxygenComp->GetOxygenLevel(), OxygenComp->GetMaxOxygenLevel());
		}

		// 스태미나
		if (UStaminaComponent* StaminaComp = Character->FindComponentByClass<UStaminaComponent>())
		{
			StaminaComp->OnStaminaChanged.AddDynamic(this, &UPlayerHUDComponent::UpdateStaminaHUD);
			UpdateStaminaHUD(StaminaComp->GetStamina(), StaminaComp->GetMaxStamina());
		}

		if (UDepthComponent* DepthComp = Character->FindComponentByClass<UDepthComponent>())
		{
			BindDepthWidgetFunction(DepthComp);
		}

		Character->OnEnvironmentStateChangedDelegate.AddDynamic(this, &UPlayerHUDComponent::UpdateEnvironmentState);
		UpdateEnvironmentState(Character->GetEnvironmentState(), Character->GetEnvironmentState());
	}

	if (ResultScreenWidgetClass)
	{
		ResultScreenWidget = CreateWidget<UResultScreen>(PlayerController, ResultScreenWidgetClass);
	}

	if (MissionsOnHUDWidgetClass)
	{
		MissionsOnHUDWidget = CreateWidget<UMissionsOnHUDWidget>(PlayerController, MissionsOnHUDWidgetClass);
		MissionsOnHUDWidget->AddToViewport();
	}

	if (Radar2DWidgetClass)
	{
		Radar2DWidget = CreateWidget<URadar2DWidget>(PlayerController, Radar2DWidgetClass);
		Radar2DWidget->AddToViewport(-1);
		SetActiveRadarWidget(false);
	}

	if (SpectatorHUDWidgetClass)
	{
		SpectatorHUDWidget = CreateWidget<USpectatorHUDWidget>(PlayerController, SpectatorHUDWidgetClass);
		if (SpectatorHUDWidget)
		{
			SpectatorHUDWidget->BindWidget(PlayerController);
		}
	}

	BindGameState();
}

void UPlayerHUDComponent::C_ShowResultScreen_Implementation()
{
	int32 TeamMaxKill = 0;
	int32 TeamMaxDamage = 0;
	int32 TeamMaxAssist = 0;

	for (AADPlayerState* PS : TActorRange<AADPlayerState>(GetWorld()))
	{
		if (PS->GetDamage() > TeamMaxDamage)
		{
			TeamMaxDamage = PS->GetDamage();
		}
		if (PS->GetMonsterKillCount() > TeamMaxKill)
		{
			TeamMaxKill = PS->GetMonsterKillCount();
		}
		if (PS->GetAssists() > TeamMaxAssist)
		{
			TeamMaxAssist = PS->GetAssists();
		}
	}

	TArray<FResultScreenParams> ResultParamsArray;

	int32 MaxCollect = 1;
	int32 MaxCombat = 1;
	int32 MaxSupport = 1;

	for (AADPlayerState* PS : TActorRange<AADPlayerState>(GetWorld()))
	{
		EAliveInfo AliveInfo = EAliveInfo::Alive;

		if (PS->IsSafeReturn() == false)
		{
			AliveInfo = (PS->IsDead()) ? EAliveInfo::Dead : EAliveInfo::Abandoned;
		}

		float DamageNomalize = (TeamMaxDamage == 0) ? 0 : ((float)PS->GetDamage() / (float)TeamMaxDamage);
		float KillNomalize = (TeamMaxKill == 0) ? 0 : ((float)PS->GetTotalMonsterKillCount() / (float)TeamMaxKill);
		float AssistNomalize = (TeamMaxAssist == 0) ? 0 : ((float)PS->GetAssists() / (float)TeamMaxAssist);

		int32 BattleContribution = 10000 * (0.6* DamageNomalize + 0.3* KillNomalize + 0.1* AssistNomalize);
		int32 SafeContribution = 100 * (PS->GetGroggyRevive() + PS->GetCorpseRecovery() * 3);

		FResultScreenParams Params
		(
			PS->GetPlayerNickname(),
			AliveInfo,
			PS->GetOreCollectedValue(), //채집 기여
			BattleContribution,//전투기여
			SafeContribution //팀지원
		);

		MaxCollect = FMath::Max(MaxCollect, Params.CollectionScore);
		MaxCombat = FMath::Max(MaxCombat, Params.BattleScore);
		MaxSupport = FMath::Max(MaxSupport, Params.SupportScore);

		ResultParamsArray.Add(Params);


		UpdateResultScreen(PS->GetPlayerIndex(), Params);
	}

	for (FResultScreenParams& Param : ResultParamsArray)
	{
		Param.NormalizedCollectScore = (float)Param.CollectionScore / (float)MaxCollect;
		Param.NormalizedCombatScore = (float)Param.BattleScore / (float)MaxCombat;
		Param.NormalizedSupportScore = (float)Param.SupportScore / (float)MaxSupport;

		float TotalScore = Param.NormalizedCollectScore + Param.NormalizedCombatScore + Param.NormalizedSupportScore;
		if (Param.AliveInfo == EAliveInfo::Alive) // 생존 보너스
		{
			Param.MVPScore = TotalScore * 1.5f;
		}
		else
		{
			Param.MVPScore = TotalScore;
		}
	}

	ResultParamsArray.Sort([](const FResultScreenParams& A, const FResultScreenParams& B)
		{
			if (!FMath::IsNearlyEqual(A.MVPScore, B.MVPScore))
			{
				return A.MVPScore > B.MVPScore;
			}

			// 동점일 경우 우선순위
			if (A.CollectionScore != B.CollectionScore)
				return A.CollectionScore > B.CollectionScore;

			if (A.BattleScore != B.BattleScore)
				return A.BattleScore > B.BattleScore;

			if (A.SupportScore != B.SupportScore)
				return A.SupportScore > B.SupportScore;

			// 마지막으로 생존자 우선
			return A.AliveInfo == EAliveInfo::Alive && B.AliveInfo != EAliveInfo::Alive;
		});

	for (int32 i = 0; i < ResultParamsArray.Num(); ++i)
	{
		UpdateResultScreen(i, ResultParamsArray[i]);
	}

	AADInGameState* GS = Cast<AADInGameState>(GetWorld()->GetGameState());
	if (GS == nullptr)
	{
		LOGV(Error, TEXT("GS == nullptr"));
		return;
	}

	ResultScreenWidget->ChangeTeamMoneyText(GS->GetTotalTeamCredit());
	SetResultScreenVisible(true);
}

void UPlayerHUDComponent::M_SetSpearUIVisibility_Implementation(bool bVisible)
{
	if (PlayerStatusWidget)
	{
		PlayerStatusWidget->SetSpearVisibility(bVisible);
	}
}

void UPlayerHUDComponent::M_UpdateSpearCount_Implementation(const int32& CurrentSpear, const int32& TotalSpear)
{
	if (PlayerStatusWidget)
	{
		PlayerStatusWidget->SetSpearCount(CurrentSpear, TotalSpear);
	}
}

void UPlayerHUDComponent::UpdateMissionsOnHUD(EMissionType MissionType, uint8 MissionIndex, int32 CurrentProgress)
{
	MissionsOnHUDWidget->UpdateMission(MissionType, MissionIndex, CurrentProgress);
}

void UPlayerHUDComponent::PlayNextPhaseAnim(int32 NextPhaseNumber)
{
	PlayerStatusWidget->PlayNextPhaseAnim(NextPhaseNumber);

	USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
	if (SoundSubsystem == nullptr)
	{
		return;
	}

	SoundSubsystem->Play2D(ESFX_UI::PhaseTransition);
}

void UPlayerHUDComponent::SetCurrentPhaseOverlayVisible(bool bShouldVisible)
{
	if (IsValid(PlayerStatusWidget) == false)
	{
		return;
	}

	PlayerStatusWidget->SetCurrentPhaseOverlayVisible(bShouldVisible);
}

void UPlayerHUDComponent::SetMaxPhaseNumber(int32 NewMaxPhaseNumber)
{
	PlayerStatusWidget->SetMaxPhaseNumber(NewMaxPhaseNumber);
}

void UPlayerHUDComponent::BindDepthWidgetFunction(UDepthComponent* DepthComp)
{
	UDepthWidget* DepthWidget = PlayerStatusWidget->GetDepthWidget();
	if (!DepthWidget)
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("DepthWidget is nullptr"));
		return;
	}
	if (!DepthComp)
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("DepthComp is nullptr"));
		return;
	}
	
	DepthComp->OnDepthZoneChangedDelegate.AddDynamic(DepthWidget, &UDepthWidget::ApplyZoneChangeToWidget);
	DepthComp->OnDepthUpdatedDelegate.AddDynamic(DepthWidget, &UDepthWidget::SetDepthText);
}

void UPlayerHUDComponent::C_SetSpearGunTypeImage_Implementation(int8 TypeNum)
{
	PlayerStatusWidget->SetSpearGunTypeImage(TypeNum);
}

void UPlayerHUDComponent::S_ReportConfirm_Implementation(AActor* RequestInteractableActor, bool bConfirmed)
{
	if (!IsValid(RequestInteractableActor))
	{
		return;
	}

	// 현재는 단일한 Interface가 없기 때문에 Drone과 같은 구체 클래스를 통해서 호출
	if (AADDrone* Drone = Cast<AADDrone>(RequestInteractableActor))
	{
		if (bConfirmed)
		{
			Drone->ExecuteConfirmedInteraction();
		}
	}
}

void UPlayerHUDComponent::OnSpectatingStateChanged(bool bIsSpectating)
{
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("OnSpectatingStateChanged: %s / Authority : %s"), bIsSpectating ? TEXT("True") : TEXT("False"), 
		GetOwnerRole() == NM_Client ? TEXT("Host") : TEXT("Client"));

	if (bIsSpectating)
	{
		ShowSpectatorHUDWidget();
	}
	else
	{
		HideSpectatorHUDWidget();
	}
}

void UPlayerHUDComponent::SetTestHUDVisibility(const bool NewVisible) const
{
	if (HudWidget)
	{
		HudWidget->SetVisibility(NewVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDComponent::ToggleTestHUD() const
{
	if (HudWidget)
	{
		const bool bIsVisible = HudWidget->GetVisibility() == ESlateVisibility::Visible;
		SetTestHUDVisibility(!bIsVisible);
	}
}

void UPlayerHUDComponent::SetResultScreenVisible(const bool bShouldVisible) const
{
	if (!ResultScreenWidget)
	{
		LOGV(Error, TEXT("ResultScreenWidget == nullptr"));
		return;
	}

	if (bShouldVisible)
	{
		ResultScreenWidget->AddToViewport();
	}
	else
	{
		ResultScreenWidget->RemoveFromParent();
	}
}

void UPlayerHUDComponent::UpdateResultScreen(int32 PlayerIndexBased_1, const FResultScreenParams& Params)
{
	if (ResultScreenWidget)
	{
		ResultScreenWidget->Update(PlayerIndexBased_1, Params);
	}
}

void UPlayerHUDComponent::SetupHudWidgetToNewPawn(APawn* NewPawn, APlayerController* PlayerController)
{
	if (!IsValid(HudWidget) && HudWidgetClass)
	{
		HudWidget = CreateWidget<UPlayerHUDWidget>(PlayerController, HudWidgetClass);
	}
	if (HudWidget)
	{
		if (!HudWidget->IsInViewport())
		{
			HudWidget->AddToViewport();
		}
		HudWidget->BindWidget(NewPawn);
	}

	if (!IsValid(CrosshairWidget) && CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UCrosshairWidget>(PlayerController, CrosshairWidgetClass);
	}
	if (CrosshairWidget)
	{
		if (!CrosshairWidget->IsInViewport())
		{
			CrosshairWidget->AddToViewport();
		}
		CrosshairWidget->BindWidget(NewPawn);
	}

	if (!IsValid(PlayerStatusWidget) && PlayerStatusWidgetClass)
	{
		PlayerStatusWidget = CreateWidget<UPlayerStatusWidget>(PlayerController, PlayerStatusWidgetClass);
	}
	if (PlayerStatusWidget)
	{
		PlayerStatusWidget->AddToViewport();
		PlayerStatusWidget->SetCompassObjectWidgetVisible(true);
	}

	if (ResultScreenWidgetClass && IsValid(ResultScreenWidget) == false)
	{
		ResultScreenWidget = CreateWidget<UResultScreen>(PlayerController, ResultScreenWidgetClass);
	}

	if (MissionsOnHUDWidgetClass && IsValid(MissionsOnHUDWidget) == false)
	{
		MissionsOnHUDWidget = CreateWidget<UMissionsOnHUDWidget>(PlayerController, MissionsOnHUDWidgetClass);
	}

	if (MissionsOnHUDWidget)
	{
		MissionsOnHUDWidget->AddToViewport();
	}

	if (!IsValid(Radar2DWidget) && Radar2DWidgetClass)
	{
		Radar2DWidget = CreateWidget<URadar2DWidget>(PlayerController, Radar2DWidgetClass);
	}

	if (Radar2DWidget)
	{
		if (Radar2DWidget->IsInViewport() == false)
		{
			Radar2DWidget->AddToViewport(-1);
			SetActiveRadarWidget(false);
		}
	}

	if (AUnderwaterCharacter* UWCharacter = Cast<AUnderwaterCharacter>(NewPawn))
	{
		if (UOxygenComponent* OxygenComp = UWCharacter->GetOxygenComponent())
		{
			OxygenComp->OnOxygenLevelChanged.AddDynamic(this, &UPlayerHUDComponent::UpdateOxygenHUD);
			UpdateOxygenHUD(OxygenComp->GetOxygenLevel(), OxygenComp->GetMaxOxygenLevel());
		}

		if (UStatComponent* StatComp = UWCharacter->FindComponentByClass<UStatComponent>())
		{
			StatComp->OnHealthChanged.AddDynamic(this, &UPlayerHUDComponent::UpdateHealthHUD);
			UpdateHealthHUD(StatComp->GetCurrentHealth(), StatComp->GetMaxHealth());
		}

		// 스태미나 컴포넌트 바인딩
		if (UStaminaComponent* StaminaComp = UWCharacter->FindComponentByClass<UStaminaComponent>())
		{
			StaminaComp->OnStaminaChanged.AddDynamic(this, &UPlayerHUDComponent::UpdateStaminaHUD);
			UpdateStaminaHUD(StaminaComp->GetStamina(), StaminaComp->GetMaxStamina());
		}

		if (UDepthComponent* DepthComp = UWCharacter->FindComponentByClass<UDepthComponent>())
		{
			BindDepthWidgetFunction(DepthComp);
		}

		UWCharacter->OnEnvironmentStateChangedDelegate.AddDynamic(this, &UPlayerHUDComponent::UpdateEnvironmentState);
		UpdateEnvironmentState(UWCharacter->GetEnvironmentState(), UWCharacter->GetEnvironmentState());
	}
}

void UPlayerHUDComponent::HideHudWidget()
{
	if (HudWidget && HudWidget->IsInViewport())
	{
		HudWidget->RemoveFromParent();
	}

	if (CrosshairWidget && CrosshairWidget->IsInViewport())
	{
		CrosshairWidget->RemoveFromParent();
	}

	if (PlayerStatusWidget && PlayerStatusWidget->IsInViewport())
	{
		PlayerStatusWidget->RemoveFromParent();
	}
}

void UPlayerHUDComponent::ShowHudWidget()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;

	if (PlayerController && Pawn)
	{
		SetupHudWidgetToNewPawn(Pawn, PlayerController);
	}
}

void UPlayerHUDComponent::SetActiveRadarWidget(bool bShouldActivate)
{
	if (IsValid(Radar2DWidget) == false || Radar2DWidget->IsValidLowLevel() == false)
	{
		return;
	}

	if (bShouldActivate)
	{
		Radar2DWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		Radar2DWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerHUDComponent::C_ShowConfirmWidget_Implementation(AActor* RequestInteractableActor)
{
	UE_LOG(LogTemp,Display, TEXT("%s request Confirm Widget"), *RequestInteractableActor->GetName());

	AADPlayerController* PC = Cast<AADPlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController()) return;
	
	if (PopupWidgetClass)
	{
		if (UInteractPopupWidget* PopupWidget = CreateWidget<UInteractPopupWidget>(PC, PopupWidgetClass))
		{
			PopupWidget->AddToViewport();
			
			PC->bShowMouseCursor = true;
			
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(PopupWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);

			PopupWidget->OnPopupConfirmed.BindLambda([this, PC, RequestInteractableActor]() {
				if (!IsValid(this) || !IsValid(PC))
				{
					return;
				}
				
				this->S_ReportConfirm(RequestInteractableActor, true);
				PC->bShowMouseCursor = false;
				PC->SetInputMode(FInputModeGameOnly());
			});
			PopupWidget->OnPopupCanceled.BindLambda([this, PC]() {
				if (!IsValid(this) || !IsValid(PC))
				{
					return;
				}
				
				PC->bShowMouseCursor = false;
				PC->SetInputMode(FInputModeGameOnly());
			});
		}
	}
}

void UPlayerHUDComponent::ShowSpectatorHUDWidget()
{
	if (!SpectatorHUDWidget && SpectatorHUDWidgetClass)
	{
		AADPlayerController* PlayerController = Cast<AADPlayerController>(GetOwner());
		SpectatorHUDWidget = CreateWidget<USpectatorHUDWidget>(PlayerController, SpectatorHUDWidgetClass);
	}
	if (SpectatorHUDWidget && !SpectatorHUDWidget->IsInViewport())
	{
		SpectatorHUDWidget->AddToViewport();
	}
}

void UPlayerHUDComponent::HideSpectatorHUDWidget()
{
	if (SpectatorHUDWidget && SpectatorHUDWidget->IsInViewport())
	{
		SpectatorHUDWidget->RemoveFromParent();
	}
}

void UPlayerHUDComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// UnPossess 상황에서 변화가 필요하면 OldPawn, NewPawn의 변화를 체크해서 구현할 것
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("OnPossessedPawnChanged: OldPawn: %s, NewPawn: %s"),
		OldPawn ? *OldPawn->GetName() : TEXT("None"), 
		NewPawn ? *NewPawn->GetName() : TEXT("None"));

	// 1. 초기 생성 : 현재는 BeginPlay에서 생성
	// 2. New Pawn이 AUnderwaterCharacter일 경우 Seamless Travel 혹은 부활 상황
	// 3. New Pawn이 nullptr일 경우 UnPossess 상황, 사망 상황
	// Spectator Pawn은 Possess가 실행되지 않으므로 OnSpectatingStateChanged에서 독립적으로 관리한다.
	
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());

	if (NewPawn)
	{
		SetupHudWidgetToNewPawn(NewPawn, PlayerController);		
	}
	else
	{
		HideHudWidget();
	}
}

void UPlayerHUDComponent::UpdateOxygenHUD(float CurrentOxygenLevel, float MaxOxygenLevel)
{
	if (PlayerStatusWidget)
	{
		const float Ratio = (MaxOxygenLevel > 0.f) ? CurrentOxygenLevel / MaxOxygenLevel : 0.f;
		PlayerStatusWidget->SetOxygenPercent(Ratio);
	}
}

void UPlayerHUDComponent::UpdateHealthHUD(int32 CurrentHealth, int32 MaxHealth)
{
	if (PlayerStatusWidget)
	{
		const float Ratio = MaxHealth > 0 ? (float)CurrentHealth / MaxHealth : 0.f;
		PlayerStatusWidget->SetHealthPercent(Ratio);
	}
}

void UPlayerHUDComponent::OnShieldUseFailed()
{
	LOGV(Error, TEXT("OnShieldUseFailed Succeeded"));
	if (PlayerStatusWidget)
		PlayerStatusWidget->NoticeInfo(TEXT("Shield가 가득 찼습니다!"), FVector2D(300.0f, -160.0f));
}

void UPlayerHUDComponent::UpdateStaminaHUD(float Stamina, float MaxStamina)
{
	if (PlayerStatusWidget)
	{
		const float Ratio = MaxStamina > 0 ? Stamina / MaxStamina : 0.f;
		PlayerStatusWidget->SetStaminaPercent(Ratio);
	}
}

void UPlayerHUDComponent::UpdateEnvironmentState(EEnvironmentState OldEnvironmentState,	EEnvironmentState NewEnvironmentState)
{
	const bool bIsUnderwater = (NewEnvironmentState == EEnvironmentState::Underwater);
	if (PlayerStatusWidget)
	{
		PlayerStatusWidget->OnChangedEnvironment(bIsUnderwater);
	}
}

bool UPlayerHUDComponent::IsTestHUDVisible() const
{
	return HudWidget && HudWidget->GetVisibility() == ESlateVisibility::Visible;
}

UMissionsOnHUDWidget* UPlayerHUDComponent::GetMissionsOnHudWidget() const
{
	return MissionsOnHUDWidget;
}

USoundSubsystem* UPlayerHUDComponent::GetSoundSubsystem()
{
	UWorld* World = GetWorld();

	if (IsValid(World) == false || World->bIsTearingDown)
	{
		return nullptr;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(World);
	if (GI == nullptr)
	{
		return nullptr;
	}

	USoundSubsystem* SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	if (SoundSubsystem == nullptr)
	{
		return nullptr;
	}

	return SoundSubsystem;
}

UPlayerStatusWidget* UPlayerHUDComponent::GetPlayerStatusWidget()
{
	return PlayerStatusWidget;
}
