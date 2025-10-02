#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MissionData.h"
#include "SelectedMissionListWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class USelectedMissionSlot;
class UMissionManagerComponent;

struct FActivatedMissionInfoList;

UCLASS()
class ABYSSDIVERUNDERWORLD_API USelectedMissionListWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 생성자, 생명주기 함수
    virtual void NativeConstruct() override;

public:

    void AddElement(const FString& ElementText);
    void RemoveElementAt(const int32& Index);
    void ModifyElementAt(const FString& NewText, const int32& Index);
    void FinishElementAt(const int8& Index);

    void AddOrModifyElement(int32 ChangedIndex, const FActivatedMissionInfoList& ChangedValue);

private:

    void Refresh();

    static int32 MakeKey(EMissionType T, int32 I) { return (((int32)T) << 16) | (I & 0xFFFF); }
protected:
#pragma region Variable


    // 위젯 참조
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> VerticalBox_MissionList;

    // 미션 슬롯 위젯 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TSubclassOf<USelectedMissionSlot> SelectedMissionSlotClass;

#pragma endregion
};
