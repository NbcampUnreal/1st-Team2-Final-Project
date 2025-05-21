// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/Item/UseFunction/UseStrategy.h"
#include "EnableShield.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UEnableShield : public UUseStrategy
{
	GENERATED_BODY()
public:
	virtual void Use(AActor* Target) override;
	
};
