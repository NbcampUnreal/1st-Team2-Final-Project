#include "ADTutorialGameMode.h"
#include "Tutorial/TutorialManager.h"
#include "Character/UnderwaterCharacter.h"
#include "Framework/ADPlayerController.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADInGameState.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"

AADTutorialGameMode::AADTutorialGameMode()
{
    DefaultPawnClass = AUnderwaterCharacter::StaticClass();
    PlayerControllerClass = AADPlayerController::StaticClass();
    PlayerStateClass = AADPlayerState::StaticClass();
    GameStateClass = AADInGameState::StaticClass(); 
}

void AADTutorialGameMode::BeginPlay()
{
    Super::BeginPlay();
    InitializeTutorial();
}

void AADTutorialGameMode::InitializeTutorial()
{
    // 레벨에 배치된 TutorialManager 찾기
    for (TActorIterator<ATutorialManager> It(GetWorld()); It; ++It)
    {
        TutorialManager = *It;
        break;
    }

}
