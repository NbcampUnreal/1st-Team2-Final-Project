#include "Framework/ADPlayerController.h"

#include "ADCampGameMode.h"
#include "ADInGameState.h"
#include "ADPlayerState.h"
#include "AbyssDiverUnderWorld.h"
#include "SettingsManager.h"
#include "ADGameInstance.h"

#include "Inventory/ADInventoryComponent.h"
#include "DataRow/PhaseGoalRow.h"
#include "UI/InteractionDescriptionWidget.h"
#include "Interactable/Item/Component/ADInteractionComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "Character/ADSpectatorPawn.h"
#include "Kismet/GameplayStatics.h"

#include "UI/LoadingScreenWidget.h"

#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"
#include "Character/PlayerComponent/ShieldComponent.h"
#include "Engine/World.h"
#include "Subsystems/SoundSubsystem.h"

#include "UI/HoldInteractionWidget.h"

AADPlayerController::AADPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextAsset(TEXT("/Game/_AbyssDiver/Input/IMC_Player.IMC_Player"));
	if (MappingContextAsset.Succeeded())
	{
		DefaultMappingContext = MappingContextAsset.Object;
	}
	else
	{
		UE_LOG(AbyssDiver, Warning, TEXT("Failed to load InputMappingContext"));
	}

	PlayerHUDComponent = CreateDefaultSubobject<UPlayerHUDComponent>(TEXT("PlayerHUDComponent"));
}

void AADPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AADPlayerController::SetPawn(APawn* InPawn)
{
	if (APawn* PreviousPawn = GetPawn())
	{
		if (AUnderwaterCharacter* OldDiver = Cast<AUnderwaterCharacter>(PreviousPawn))
		{
			if (UADInteractionComponent* OldInteractionComp = OldDiver->GetInteractionComponent())
			{
				OldInteractionComp->OnFocus.RemoveAll(InteractionWidget);
				OldInteractionComp->OnFocusEnd.RemoveAll(InteractionWidget);

				OldInteractionComp->OnHoldStart.RemoveAll(InteractionHoldWidget);
				OldInteractionComp->OnHoldCancel.RemoveAll(InteractionHoldWidget);
			}
		}
	}

	Super::SetPawn(InPawn);

	if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetPawn()))
	{
		if (UADInteractionComponent* InteractionComponent = UnderwaterCharacter->GetInteractionComponent())
		{
			if (InteractionWidgetClass && IsLocalController())
			{
				InteractionWidget = CreateWidget<UInteractionDescriptionWidget>(this, InteractionWidgetClass);
				if (InteractionWidget)
				{
					InteractionComponent->OnFocus.AddDynamic(InteractionWidget, &UInteractionDescriptionWidget::HandleFocus);
					InteractionComponent->OnFocusEnd.AddDynamic(InteractionWidget, &UInteractionDescriptionWidget::HandleFocusLost);
				}
			}

			if (InteractionHoldWidgetClass && IsLocalController())
			{
				InteractionHoldWidget = CreateWidget<UHoldInteractionWidget>(this, InteractionHoldWidgetClass);
				if (InteractionHoldWidget)
				{
					InteractionComponent->OnHoldStart.AddDynamic(InteractionHoldWidget, &UHoldInteractionWidget::HandleHoldStart);
					InteractionComponent->OnHoldCancel.AddDynamic(InteractionHoldWidget, &UHoldInteractionWidget::HandleHoldCancel);
				}
			}
		}
	}
}

void AADPlayerController::PostNetInit()
{
	Super::PostNetInit();
	LOGVN(Log, TEXT("PostNetInit"));
	OnPostNetInit();

	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AADPlayerController::PostSeamlessTravel()
{
	LOGVN(Log, TEXT("Post SeamlessTravel"));

	Super::PostSeamlessTravel();

	OnPostSeamlessTravel();
}

void AADPlayerController::C_OnPreClientTravel_Implementation()
{
	OnPreClientTravel();
	ShowFadeOut();
}

void AADPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (IsInState(NAME_Spectating))
	{
		ServerViewNextPlayer();
	}
}

void AADPlayerController::StartSpectate()
{
	if (!HasAuthority())
	{
		return;
	}

	// Change State를 하면 BeginSpectate에 의해서 SpectatorPawn이 생성된다.
	// PlayerState Spectator 설정은 EndSpectateState에서 처리된다.
	PlayerState->SetIsSpectator(true);
	ChangeState(NAME_Spectating);
	bPlayerIsWaiting = true;

	ClientGotoState(NAME_Spectating);

	ViewAPlayer(1);
}

void AADPlayerController::SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
{
	Super::SetViewTarget(NewViewTarget, TransitionParams);

	// UE_LOG(LogTemp,Display, TEXT("SetViewTarget called for %s, NewViewTarget: %s"), *GetName(), NewViewTarget ? *NewViewTarget->GetName() : TEXT("None"));
	// UE_LOG(LogTemp,Display, TEXT("Transition Params : BlendTime: %f, BlendFunction: %d, BlendExp: %f, bLockOutgoing: %s"),
	// 	TransitionParams.BlendTime, (int32)TransitionParams.BlendFunction, TransitionParams.BlendExp, TransitionParams.bLockOutgoing ? TEXT("True") : TEXT("False"));
	//
	// UE_LOG(LogTemp,Display,	TEXT("Player CameraManager Client Simulating View Target : %s"), 
	// 	PlayerCameraManager->bUseClientSideCameraUpdates ? TEXT("True") : TEXT("False"));
	OnTargetViewChanged.Broadcast(GetViewTarget());
}

void AADPlayerController::C_StartCameraBlink_Implementation(FColor FadeColor, FVector2D FadeAlpha, float FadeStartTime, float FadeEndDelay, float FadeEndTime)
{
	if (PlayerCameraManager != nullptr)
	{
		const float BlankFadeStartAlpha = FadeAlpha.X >= 0.0f ? FadeAlpha.X : PlayerCameraManager->FadeAmount;
		const float BlankFadeEndAlpha = FadeAlpha.Y;
		if (FadeStartTime > 0.0f)
		{
			PlayerCameraManager->StartCameraFade(BlankFadeStartAlpha, BlankFadeEndAlpha, FadeStartTime, FadeColor.ReinterpretAsLinear(), false, true);
		}
		else
		{
			PlayerCameraManager->SetManualCameraFade(BlankFadeEndAlpha, FadeColor.ReinterpretAsLinear(), false);
		}

		TWeakObjectPtr WeakActor = this;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindWeakLambda(this, [=]()
		{
			if (WeakActor.IsValid())
			{
				if (WeakActor->PlayerCameraManager != nullptr)
				{
					WeakActor->PlayerCameraManager->StartCameraFade(BlankFadeEndAlpha, BlankFadeStartAlpha, FadeEndTime, FadeColor.ReinterpretAsLinear(), false, true);
				}
			}
		});
		GetWorldTimerManager().SetTimer(
			CameraBlankTimerHandle,
			TimerDelegate,
			FadeStartTime + FadeEndDelay,
			false
		);
	}
}

bool AADPlayerController::IsCameraBlanking() const
{
	return GetWorldTimerManager().IsTimerActive(CameraBlankTimerHandle) || (PlayerCameraManager && PlayerCameraManager->bEnableFading);
}

void AADPlayerController::ShowPlayerHUD()
{
	if (PlayerHUDComponent)
	{
		PlayerHUDComponent->ShowHudWidget();
	}
}

void AADPlayerController::HidePlayerHUD()
{
	if (PlayerHUDComponent)
	{
		PlayerHUDComponent->HideHudWidget();
	}
}

void AADPlayerController::SetInvincible(bool bIsInvincible)
{
	if (HasAuthority())
	{
		if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetPawn()))
		{
			UnderwaterCharacter->SetInvincible(bIsInvincible);
		}
	}
	else
	{
		S_SetInvincible(bIsInvincible);
	}
}

void AADPlayerController::KillPlayer()
{
	if (HasAuthority())
	{
		if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetPawn()))
		{
			UnderwaterCharacter->Kill();
		}
	}
	else
	{
		S_KillPlayer();
	}
}

void AADPlayerController::S_SetInvincible_Implementation(bool bIsInvincible)
{
	SetInvincible(bIsInvincible);
}

void AADPlayerController::C_PlaySound_Implementation(ESFX SoundType, float VolumeMultiplier, float PitchMultiplier)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USoundSubsystem* SoundSubsystem = GameInstance->GetSubsystem<USoundSubsystem>())
		{
			SoundSubsystem->Play2D(SoundType, VolumeMultiplier);
		}
	}
}

void AADPlayerController::S_KillPlayer_Implementation()
{
	KillPlayer();
}

void AADPlayerController::BeginSpectatingState()
{
	UE_LOG(LogAbyssDiverSpectate, Display, TEXT("Begin Spectating State for %s, GetPawn : %s"), *GetName(), GetPawn() ? *GetPawn()->GetName() : TEXT("None"));
	
	Super::BeginSpectatingState();

	OnSpectateChanged.Broadcast(true);
}

void AADPlayerController::EndSpectatingState()
{
	UE_LOG(LogAbyssDiverSpectate, Display, TEXT("End Spectating State for %s"), *GetName());
	
	
	Super::EndSpectatingState();

	OnSpectateChanged.Broadcast(false);
}

void AADPlayerController::S_RequestSelectLevel_Implementation(const EMapName InLevelName)
{
	if (HasAuthority())
	{
		if (AADCampGameMode* ADGameMode = Cast<AADCampGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			ADGameMode->SetSelectedLevel(InLevelName);
		}
	}
}

void AADPlayerController::S_RequestStartGame_Implementation()
{
	if (HasAuthority())
	{
		if (AADCampGameMode* ADGameMode = Cast<AADCampGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			ADGameMode->TryStartGame();
		}
	}
}

void AADPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InventoryAction)
		{
			EnhancedInput->BindAction(
				InventoryAction,
				ETriggerEvent::Started,
				this,
				&AADPlayerController::ShowInventory
			);
			EnhancedInput->BindAction(
				InventoryAction,
				ETriggerEvent::Completed,
				this,
				&AADPlayerController::HideInventory
			);
		}
	}
}

void AADPlayerController::ShowInventory(const FInputActionValue& InputActionValue)
{
	AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetPawn());
	if (UnderwaterCharacter)
	{
		if (UnderwaterCharacter->IsNormal())
		{
			if (AADPlayerState* PS = GetPlayerState<AADPlayerState>())
			{
				PS->GetInventory()->ShowInventory();
			}
		}
	}
}

void AADPlayerController::HideInventory(const FInputActionValue& InputActionValue)
{
	AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetPawn());
	if (UnderwaterCharacter)
	{
		if (UnderwaterCharacter->IsNormal())
		{
			if (AADPlayerState* PS = GetPlayerState<AADPlayerState>())
			{
				PS->GetInventory()->HideInventory();
			}
		}
	}
}

void AADPlayerController::ToggleTestHUD()
{
	if (PlayerHUDComponent)
	{
		PlayerHUDComponent->ToggleTestHUD();
	}
}

void AADPlayerController::GainShield(int Amount)
{
	if (HasAuthority())
	{
		if (UShieldComponent* ShieldComponent = GetPawn() ? GetPawn()->FindComponentByClass<UShieldComponent>() : nullptr)
		{
			ShieldComponent->GainShield(Amount);
		}
	}
	else
	{
		S_GainShield(Amount);
	}
}

void AADPlayerController::S_GainShield_Implementation(int Amount)
{
	GainShield(Amount);
}

void AADPlayerController::C_PlayGameOverSound_Implementation()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USoundSubsystem* SoundSubsystem = GameInstance->GetSubsystem<USoundSubsystem>())
		{
			SoundSubsystem->PlayAmbient(ESFX_Ambient::AllPlayersDie);
		}
	}
}

void AADPlayerController::ShowFadeOut(float Duration)
{
	if (PlayerCameraManager == nullptr)
	{
		LOGVN(Log, TEXT("There is no PlayerChameraManager"));
		return;
	}

	if (IsLocalController() == false)
	{
		LOGVN(Log, TEXT("Not Local Controller"));
		return;
	}

	PlayerCameraManager->StartCameraFade(
		0.f,
		1.f,
		Duration,
		FLinearColor::Black,
		false,
		true
	);
}

void AADPlayerController::ShowFadeIn()
{
	if (PlayerCameraManager == nullptr)
	{
		LOGVN(Log, TEXT("There is no PlayerChameraManager"));
		return;
	}

	if (IsLocalController() == false)
	{
		LOGVN(Log, TEXT("Not Local Controller"));
		return;
	}

	PlayerCameraManager->StartCameraFade(
		1.f,
		0.0f,
		3.0f,
		FLinearColor::Black,
		false,
		false
	);
}
void AADPlayerController::OnCameraBlankEnd()
{
	
}
