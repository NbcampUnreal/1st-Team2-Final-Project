// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/UseFunction/EnableShield.h"
#include "AbyssDiverUnderWorld.h"
#include "Framework/ADPlayerState.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/PlayerComponent/ShieldComponent.h"
#include "Character/PlayerComponent/CombatEffectComponent.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "DataRow/FADItemDataRow.h"


bool UEnableShield::Use(AActor* Target)
{

	LOG(TEXT("UseShield"));

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* Row = DataTableSubsystem->GetItemData(0);
		APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(Target)->GetPlayerController());
		if (Row && PC)
		{
			if (AUnderwaterCharacter* UnderwaterCharacter = PC->GetPawn<AUnderwaterCharacter>())
			{
				if (UShieldComponent* ShieldComp = UnderwaterCharacter->GetShieldComponent())
				{
					if (ShieldComp->IsShieldFull())
					{
						// Shield 가득 찼을 때 효과음이 있다면 재생 추가
						return false;
					}
					else
					{
						ShieldComp->GainShield(Row->Amount);
						if (UCombatEffectComponent* CombatEffectComp = UnderwaterCharacter->GetCombatEffectComponent())
						{
							CombatEffectComp->C_PlayShieldUseEffect();
						}
						return true;
					}
				}
			}
		}
	}

	return false;
}
