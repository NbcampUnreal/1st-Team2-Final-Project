#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "WaitForSomethingWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UWaitForSomethingWidget : public UUserWidget
{
	GENERATED_BODY()
	

#pragma region Methods

public:

	void SetWaitText(const FString& NewText);

#pragma endregion


#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaitText;

#pragma endregion

};
