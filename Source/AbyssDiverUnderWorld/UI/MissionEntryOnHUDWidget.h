#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "MissionEntryOnHUDWidget.generated.h"

class UImage;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UMissionEntryOnHUDWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Methods

public:

	void ChangeImage(UTexture2D* Image);
	void UpdateMissionEntryColor(bool bIsMissionCompleted);
	void SetVisible(bool bShouldVisible);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MissionEntryImage;

	static const FLinearColor CompleteColor;
	static const FLinearColor IncompleteColor;

#pragma endregion
	
};
