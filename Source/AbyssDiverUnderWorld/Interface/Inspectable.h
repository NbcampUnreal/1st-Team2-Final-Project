// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Inspectable.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UInspectable : public UInterface
{
	GENERATED_BODY()
};

class ABYSSDIVERUNDERWORLD_API IInspectable
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Dex")
	FName GetMonsterId() const;
};
