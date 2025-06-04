// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/UseFunction/EnableShield.h"
#include "AbyssDiverUnderWorld.h"
#include "Framework/ADPlayerState.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/PlayerComponent/ShieldComponent.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "DataRow/FADItemDataRow.h"

void UEnableShield::Use(AActor* Target)
{

	LOG(TEXT("UseShield"));

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* Row = DataTableSubsystem->GetItemData(0);
		APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(Target)->GetPlayerController());
		if (Row && PC)
		{
			APawn* OwnerPawn = PC->GetPawn();
			if (OwnerPawn)
			{
				AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(OwnerPawn);
				UShieldComponent* ShieldComp = UnderwaterCharacter->GetShieldComponent();
				if (ShieldComp)
				{
					ShieldComp->GainShield(Row->Amount);
				}
			}
		}
	}
}
