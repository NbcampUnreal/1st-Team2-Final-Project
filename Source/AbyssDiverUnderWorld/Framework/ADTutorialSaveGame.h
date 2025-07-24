// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ADTutorialSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UADTutorialSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UADTutorialSaveGame();

	UPROPERTY()
	uint8 bHasCompletedTutorial : 1;

	static const FString SaveSlotName;
	static const uint32 UserIndex;
	
};
