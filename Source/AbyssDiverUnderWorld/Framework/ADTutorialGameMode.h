#pragma once

#include "CoreMinimal.h"
#include "Framework/ADInGameMode.h"
#include "Tutorial/TutorialManager.h"
#include "ADTutorialGameMode.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTutorialGameMode : public AADInGameMode
{
    GENERATED_BODY()

    public:
    AADTutorialGameMode();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    ATutorialManager* GetTutorialManager() const { return TutorialManager; }

protected:
    virtual void BeginPlay() override;

    void InitializeTutorial();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ATutorialManager> TutorialManager;
};
