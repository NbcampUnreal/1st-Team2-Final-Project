// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/UseFunction/RecoveryOxygen.h"
#include "AbyssDiverUnderWorld.h"
#include "Framework/ADPlayerState.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/PlayerComponent/OxygenComponent.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "DataRow/FADItemDataRow.h"

void URecoveryOxygen::Use(AActor* Target)
{
	LOG(TEXT("RecoveryOxygen"));
	//TODO: 산소 회복 이펙트
	//TODO: 산소 회복 사운드

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* Row = DataTableSubsystem->GetItemDataByName("OxygenPack");
		APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(Target)->GetPlayerController());
		if (Row && PC)
		{
			APawn* OwnerPawn = PC->GetPawn();
			if (OwnerPawn)
			{
				AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(OwnerPawn);
				UOxygenComponent* OxygenComp = UnderwaterCharacter->GetOxygenComponent();
				if (OxygenComp)
				{
					OxygenComp->RefillOxygen(Row->Amount);
				}
			}
		}
	}
}
