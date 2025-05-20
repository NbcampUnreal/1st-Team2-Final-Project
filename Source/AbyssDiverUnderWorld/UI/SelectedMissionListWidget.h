#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MissionData.h"
#include "SelectedMissionListWidget.generated.h"

class UVerticalBox;
class UMissionEntryWidget;

UCLASS()
class ABYSSDIVERUNDERWORLD_API USelectedMissionListWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 생성자, 생명주기 함수
    virtual void NativeConstruct() override;

protected:
#pragma region Variable
    // 위젯 참조
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> VerticalBox_MissionList;

    // 미션 엔트리 위젯 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TSubclassOf<UMissionEntryWidget> MissionEntryWidgetClass;
#pragma endregion
};
