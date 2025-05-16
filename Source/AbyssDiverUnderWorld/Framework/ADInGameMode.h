#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ADInGameMode.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADInGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AADInGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	void InitPlayer(APlayerController* PC);
	

};
