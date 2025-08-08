#include "Framework/MainMenuGameMode.h"

#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/Localizations/LocalizationSubsystem.h"

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	const int32 InitPoolCount = 1;
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->Init(InitPoolCount);
	GetGameInstance()->GetSubsystem<USoundSubsystem>()->PlayBGM(ESFX_BGM::Sound0);
	
	// 테스트용 코드, Standalone으로 실행시 번역 적용
	if (GetWorld()->WorldType == EWorldType::Game)
	{
		GetGameInstance()->GetSubsystem<ULocalizationSubsystem>()->SetLanguage(ELanguage::Korean);
	}
}
