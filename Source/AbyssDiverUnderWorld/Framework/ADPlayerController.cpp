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
#include "Kismet/GameplayStatics.h"
#include "UI/LoadingScreenWidget.h"

#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"
#include "Engine/World.h"

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
			}
		}
	}

	Super::SetPawn(InPawn);

	if (InPawn)
	{
		if (InteractionWidgetClass && IsLocalController())
		{
			InteractionWidget = CreateWidget<UInteractionDescriptionWidget>(this, InteractionWidgetClass);

			if (InteractionWidget)
			{
				if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetPawn()))
				{
					if (UADInteractionComponent* InteractionComponent = UnderwaterCharacter->GetInteractionComponent())
					{
						InteractionComponent->OnFocus.AddDynamic(InteractionWidget, &UInteractionDescriptionWidget::HandleFocus);
						InteractionComponent->OnFocusEnd.AddDynamic(InteractionWidget, &UInteractionDescriptionWidget::HandleFocusLost);
					}
				}
			}
		}
	}
}

void AADPlayerController::PostNetInit()
{
	Super::PostNetInit();

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
	LOG(TEXT("Post SeamlessTravel"));
	Super::PostSeamlessTravel();
	OnPostSeamlessTravel();
	FTimerHandle FadeInTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		FadeInTimerHandle,
		this,
		&AADPlayerController::ShowFadeIn,
		2.0f, 
		false
	);

}

void AADPlayerController::C_OnPreClientTravel_Implementation()
{
	OnPreClientTravel();
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

void AADPlayerController::C_ShowFadeOut_Implementation(float Duration)
{
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(
			0.f,                        // From: 투명
			1.f,                        // To: 검정
			Duration,
			FLinearColor::Black,
			false,                      // 오디오 페이드 X
			true                        // 유지 (검정 상태로 고정)
		);
	}
}

void AADPlayerController::ShowFadeIn()
{
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(
			1.f,                        // From: 검정
			0.0f,                        // To: 투명
			3.0f,
			FLinearColor::Black,
			false,
			false                      // 유지 X → 다시 원래대로
		);
	}

}
