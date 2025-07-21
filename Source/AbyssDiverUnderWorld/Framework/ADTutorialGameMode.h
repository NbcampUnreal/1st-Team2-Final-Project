
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

    /** ���� �� TutorialManager �ʱ�ȭ �� ���� */
    void InitializeTutorial();

public:
    /** TutorialManager ���ٿ� Getter */
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    ATutorialManager* GetTutorialManager() const { return TutorialManager; }

protected:
    /** ���� ������ �����ϴ� TutorialManager */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ATutorialManager> TutorialManager;

};
