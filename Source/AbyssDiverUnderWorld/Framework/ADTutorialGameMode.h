
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

protected:
    virtual void BeginPlay() override;

    /** 레벨 내 TutorialManager 초기화 및 시작 */
    void InitializeTutorial();

public:
    /** TutorialManager 접근용 Getter */
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    ATutorialManager* GetTutorialManager() const { return TutorialManager; }

protected:
    /** 현재 레벨에 존재하는 TutorialManager */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ATutorialManager> TutorialManager;

};
