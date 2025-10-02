#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SelectedMissionSlot.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class ABYSSDIVERUNDERWORLD_API USelectedMissionSlot : public UUserWidget
{
	GENERATED_BODY()
#pragma region Method
public:
	void OnMissionFinished();
	void SetMissionTitle(const FString& Title);

	void SetProgressText(int32 Current, int32 Goal);
	void SetCompleted(bool bCompleted) { if (bCompleted) { OnMissionFinished(); } }
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Title;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MissionBG;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FinishLineImage;
#pragma endregion

#pragma region getter, setter
public:

#pragma endregion
};
