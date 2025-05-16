#include "Framework/ADPlayerController.h"

#include "AbyssDiverUnderWorld.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "ADPlayerState.h"
#include "EnhancedInputComponent.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Inventory/ADInventoryComponent.h"

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

void AADPlayerController::S_SetPlayerInfo_Implementation(const FUniqueNetIdRepl& Id, const FString& Nickname)
{
	if (AADPlayerState* PS = GetPlayerState<AADPlayerState>())
	{
		PS->SetPlayerInfo(Id, Nickname);
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
				&AADPlayerController::ToggleInventoryShowed
			);
		}
	}
}

void AADPlayerController::ToggleInventoryShowed(const FInputActionValue& InputActionValue)
{
	if (AADPlayerState* PS = GetPlayerState<AADPlayerState>())
	{
		PS->GetInventory()->ToggleInventoryShowed();
	}
}
