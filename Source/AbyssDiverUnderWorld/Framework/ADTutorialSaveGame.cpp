// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADTutorialSaveGame.h"

const FString UADTutorialSaveGame::SaveSlotName = TEXT("TutorialSaveSlot");
const uint32 UADTutorialSaveGame::UserIndex = 0;

UADTutorialSaveGame::UADTutorialSaveGame()
{
	bHasCompletedTutorial = false;
}