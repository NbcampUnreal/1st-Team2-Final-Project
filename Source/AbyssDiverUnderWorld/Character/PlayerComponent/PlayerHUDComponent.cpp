#include "PlayerHUDComponent.h"

#include "AbyssDiverUnderWorld.h"
#include "Character/PlayerHUDWidget.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/PlayerComponent/OxygenComponent.h"
#include "Character/PlayerComponent/StaminaComponent.h"
#include "Character/StatComponent.h"
#include "Framework/ADInGameState.h"
#include "Framework/ADPlayerState.h"
#include "UI/ResultScreen.h"
#include "UI/PlayerStatusWidget.h"
#include "UI/MissionsOnHUDWidget.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Components/CanvasPanel.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/CrosshairWidget.h"

UPlayerHUDComponent::UPlayerHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	PlayerController->OnPossessedPawnChanged.AddDynamic(this, &UPlayerHUDComponent::OnPossessedPawnChanged);
	
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

	// 올바른 수정
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
}

void UPlayerHUDComponent::C_ShowResultScreen_Implementation()
{
	for (AADPlayerState* PS : TActorRange<AADPlayerState>(GetWorld()))
	{
		AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(PS->GetPawn());
		if (ensure(PlayerCharacter) == false)
		{
			continue;
		}

		EAliveInfo AliveInfo = EAliveInfo::Alive;

		if (PS->IsSafeReturn() == false)
		{
			const ECharacterState CharacterState = PlayerCharacter->GetCharacterState();
			switch (CharacterState)
			{
			case ECharacterState::Normal:
				AliveInfo = EAliveInfo::Abandoned;
				break;
			case ECharacterState::Groggy:
				AliveInfo = EAliveInfo::Dead;
				break;
			case ECharacterState::Death:
				AliveInfo = EAliveInfo::Dead;
				break;
			default:
				check(false);
				break;
			}
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

void UPlayerHUDComponent::UpdateMissionsOnHUD(EMissionType MissionType, uint8 MissionIndex, int32 CurrentProgress)
{
	MissionsOnHUDWidget->UpdateMission(MissionType, MissionIndex, CurrentProgress);
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

void UPlayerHUDComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// UnPossess 상황에서 변화가 필요하면 OldPawn, NewPawn의 변화를 체크해서 구현할 것
	
	if (!NewPawn) return;

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());

	if (!IsValid(HudWidget))
	{
		HudWidget = CreateWidget<UPlayerHUDWidget>(PlayerController, HudWidgetClass);
	}
	if (HudWidget)
	{
		HudWidget->AddToViewport();
		HudWidget->BindWidget(NewPawn);
	}

	if (!IsValid(CrosshairWidget))
	{
		CrosshairWidget = CreateWidget<UCrosshairWidget>(PlayerController, CrosshairWidgetClass);
	}
	if (CrosshairWidget)
	{
		CrosshairWidget->AddToViewport();
		CrosshairWidget->BindWidget(NewPawn);
	}

	if (!IsValid(PlayerStatusWidget))
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

void UPlayerHUDComponent::UpdateStaminaHUD(float Stamina, float MaxStamina)
{
	if (PlayerStatusWidget)
	{
		const float Ratio = MaxStamina > 0 ? Stamina / MaxStamina : 0.f;
		PlayerStatusWidget->SetStaminaPercent(Ratio);
	}
}

void UPlayerHUDComponent::UpdateSpearCount(const int32& CurrentSpear, const int32& TotalSpear)
{
	if (PlayerStatusWidget)
	{
		PlayerStatusWidget->SetSpearCount(CurrentSpear, TotalSpear);
	}
}

void UPlayerHUDComponent::SetSpearUIVisibility(bool bVisible)
{
	if (PlayerStatusWidget)
	{
		PlayerStatusWidget->SetSpearVisibility(bVisible);
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
