// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/UseFunction/RecoveryOxygen.h"
#include "AbyssDiverUnderWorld.h"
#include "Framework/ADPlayerState.h"
#include "Character/UnderwaterCharacter.h"

void URecoveryOxygen::Use(AActor* Target)
{
	LOG(TEXT("RecoveryOxygen"));
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(Target)->GetPlayerController());
	if (PC)
	{
		APawn* OwnerPawn = PC->GetPawn();
		if (OwnerPawn)
		{
			AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(OwnerPawn);
			//UnderwaterCharacter-> OxygenComponent getter 요청
		}
	}
}
