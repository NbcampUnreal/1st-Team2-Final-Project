#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "RadarReturnWidget.generated.h"

class UOverlay;
class UImage;
/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API URadarReturnWidget : public UUserWidget
{
	GENERATED_BODY()

protected:


#pragma region Methods

public:

	void SetReturnImageColor(FColor NewColor);

#pragma endregion


#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> RadarReturnOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RadarReturnImage;

#pragma endregion

	
};
