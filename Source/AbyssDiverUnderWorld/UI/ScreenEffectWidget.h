// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScreenEffectWidget.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UScreenEffectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    /*0~1의 값 지정*/
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SetEffectAmount(float Amount, float MaxAmount);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    UWidgetAnimation* GetMainAnim();

};
