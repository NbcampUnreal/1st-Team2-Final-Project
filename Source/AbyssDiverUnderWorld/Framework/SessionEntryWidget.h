#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionListWidget.h"
#include "SessionEntryWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class ABYSSDIVERUNDERWORLD_API USessionEntryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SessionName;

};
