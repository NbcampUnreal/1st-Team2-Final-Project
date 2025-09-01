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
#include "UI/DepthWidget.h"

UPlayerHUDComponent::UPlayerHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	AADPlayerController* PlayerController = Cast<AADPlayerController>(GetOwner());

	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	PlayerController->OnPossessedPawnChanged.AddDynamic(this, &UPlayerHUDComponent::OnPossessedPawnChanged);
	PlayerController->OnSpectateChanged.AddDynamic(this, &UPlayerHUDComponent::OnSpectatingStateChanged);
	
	// 메인 HUD 생성
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

	if (CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UCrosshairWidget>(PlayerController, CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport();
			CrosshairWidget->BindWidget(PlayerController->GetPawn());
		}
	}

	// 상태 UI 생성
	if (PlayerStatusWidgetClass)
	{
		PlayerStatusWidget = CreateWidget<UPlayerStatusWidget>(PlayerController, PlayerStatusWidgetClass);
		if (PlayerStatusWidget)
		{
			PlayerStatusWidget->AddToViewport();
			PlayerStatusWidget->SetCompassObjectWidgetVisible(true);
		}
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


	if (APawn* Pawn = PlayerController->GetPawn())
	{
		if (AUnderwaterCharacter* UWCharacter = Cast<AUnderwaterCharacter>(Pawn)) 
		{
			// 스탯 컴포넌트
			if (UStatComponent* StatComponent = UWCharacter->FindComponentByClass<UStatComponent>())
			{
				StatComponent->OnHealthChanged.AddDynamic(this, &UPlayerHUDComponent::UpdateHealthHUD);
				UpdateHealthHUD(StatComponent->GetCurrentHealth(), StatComponent->GetMaxHealth());
			}

			// 산소
			if (UOxygenComponent* OxygenComp = UWCharacter->FindComponentByClass<UOxygenComponent>())
			{
				OxygenComp->OnOxygenLevelChanged.AddDynamic(this, &UPlayerHUDComponent::UpdateOxygenHUD);
				UpdateOxygenHUD(OxygenComp->GetOxygenLevel(), OxygenComp->GetMaxOxygenLevel());
			}

			// 스태미나
			if (UStaminaComponent* StaminaComp = UWCharacter->FindComponentByClass<UStaminaComponent>())
			{
				StaminaComp->OnStaminaChanged.AddDynamic(this, &UPlayerHUDComponent::UpdateStaminaHUD);
				UpdateStaminaHUD(StaminaComp->GetStamina(), StaminaComp->GetMaxStamina());
			}
		}
	}

	if (SpectatorHUDWidgetClass)
	{
		SpectatorHUDWidget = CreateWidget<USpectatorHUDWidget>(PlayerController, SpectatorHUDWidgetClass);
		if (SpectatorHUDWidget)
		{
			SpectatorHUDWidget->BindWidget(PlayerController);
		}
	}
	
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

void UPlayerHUDComponent::C_ShowResultScreen_Implementation()
{
	for (AADPlayerState* PS : TActorRange<AADPlayerState>(GetWorld()))
	{
		EAliveInfo AliveInfo = EAliveInfo::Alive;

		if (PS->IsSafeReturn() == false)
		{
			AliveInfo = (PS->IsDead()) ? EAliveInfo::Dead : EAliveInfo::Abandoned;
		}

		FResultScreenParams Params
		(
			PS->GetPlayerNickname(),
			98,
			PS->GetOreMinedCount(),
			AliveInfo
		);

		UpdateResultScreen(PS->GetPlayerIndex(), Params);
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

void UPlayerHUDComponent::BindDeptWidgetFunction(UDepthComponent* DepthComp)
{
	UDepthWidget* DepthWidget = PlayerStatusWidget->GetDepthWidget();
	if (!DepthWidget || !DepthComp) return;
	DepthComp->OnDepthZoneChangedDelegate.AddDynamic(DepthWidget, &UDepthWidget::ApplyZoneChangeToWidget);
	DepthComp->OnDepthUpdatedDelegate.AddDynamic(DepthWidget, &UDepthWidget::SetDepthText);
}

void UPlayerHUDComponent::C_SetSpearGunTypeImage_Implementation(int8 TypeNum)
{
	PlayerStatusWidget->SetSpearGunTypeImage(TypeNum);
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
		PlayerStatusWidget->NoticeInfo(TEXT("Shield가 가득 찼습니다!"), FVector2D(0.0f, -160.0f));
}

void UPlayerHUDComponent::UpdateStaminaHUD(float Stamina, float MaxStamina)
{
	if (PlayerStatusWidget)
	{
		const float Ratio = MaxStamina > 0 ? Stamina / MaxStamina : 0.f;
		PlayerStatusWidget->SetStaminaPercent(Ratio);
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
