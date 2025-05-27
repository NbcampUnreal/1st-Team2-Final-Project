#include "Framework/MainMenuGameMode.h"

#include "Subsystems/SoundSubsystem.h"

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	const int32 InitPoolCount = 1;
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Init(InitPoolCount);
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->PlayBGM(ESFX_BGM::Sound0);
}
