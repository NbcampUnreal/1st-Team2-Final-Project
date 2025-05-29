#include "Framework/ADPlayerController.h"
#include "ADCampGameMode.h"
#include "ADInGameState.h"
#include "ADPlayerState.h"
#include "AbyssDiverUnderWorld.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Inventory/ADInventoryComponent.h"
#include "DataRow/PhaseGoalRow.h"
#include "Kismet/GameplayStatics.h"
#include "UI/InteractionDescriptionWidget.h"
#include "Character/UnderwaterCharacter.h"
#include "Interactable/Item/Component/ADInteractionComponent.h"

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

		FString Nickname = TEXT("Guest");
		FUniqueNetIdRepl Id;

		if (GetLocalPlayer())
		{
			Id = GetLocalPlayer()->GetPreferredUniqueNetId();
			Nickname = GetLocalPlayer()->GetNickname(); 
		}

		S_SetPlayerInfo(Id, Nickname);
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

	// 상호작용 UI 생성과 함수 바인딩
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

void AADPlayerController::S_SetPlayerInfo_Implementation(const FUniqueNetIdRepl& Id, const FString& Nickname)
{
	if (AADPlayerState* PS = GetPlayerState<AADPlayerState>())
	{
		PS->SetPlayerInfo(Nickname);
	}
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
	if (AADPlayerState* PS = GetPlayerState<AADPlayerState>())
	{
		PS->GetInventory()->ShowInventory();
	}
}

void AADPlayerController::HideInventory(const FInputActionValue& InputActionValue)
{
	if (AADPlayerState* PS = GetPlayerState<AADPlayerState>())
	{
		PS->GetInventory()->HideInventory();
	}
}
