#include "TutorialGameMode.h"
#include "Tutorial/TutorialManager.h"
#include "Character/UnderwaterCharacter.h"
#include "Framework/ADPlayerController.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADInGameState.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"

ATutorialGameMode::ATutorialGameMode()
{
    DefaultPawnClass = AUnderwaterCharacter::StaticClass();
    PlayerControllerClass = AADPlayerController::StaticClass();
    PlayerStateClass = AADPlayerState::StaticClass();
    GameStateClass = AADInGameState::StaticClass(); 
}

void ATutorialGameMode::BeginPlay()
{
    Super::BeginPlay();
    InitializeTutorial();
}

void ATutorialGameMode::InitializeTutorial()
{
    // ������ ��ġ�� TutorialManager ã��
    for (TActorIterator<ATutorialManager> It(GetWorld()); It; ++It)
    {
        TutorialManager = *It;
        break;
    }

}
