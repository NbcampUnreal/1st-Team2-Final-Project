#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "SavedSessionInfoWidgetData.generated.h"

class USavedSessionInfoWidget;

DECLARE_DELEGATE_OneParam(FOnInfoWidgetUpdatedDelegate, USavedSessionInfoWidget* /*UpdatedInfoWidget*/);
/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API USavedSessionInfoWidgetData : public UObject
{
	GENERATED_BODY()


#pragma region Methods

public:

	void InitData(const FString& InSavedSessionName, const FString& InSavedSessionDateText, int32 InInfoIndex);

	FOnInfoWidgetUpdatedDelegate OnInfoWidgetUpdated;

#pragma endregion

#pragma region Variables

public:

	FString SavedSessionName;
	FString SavedSessionDateText;
	int32 InfoIndex;

#pragma endregion
};
