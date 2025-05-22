#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabletBaseWidget.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UTabletBaseWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Start;
};
