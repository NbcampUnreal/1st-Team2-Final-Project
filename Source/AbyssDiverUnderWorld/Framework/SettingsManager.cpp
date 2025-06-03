#include "Framework/SettingsManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "ADGameInstance.h"
#include "Subsystems/SoundSubsystem.h"
#include "GameFramework/InputSettings.h"

USettingsManager::USettingsManager()
{
	CachedAudioSettings = { 0.5, 0.5, 0.5, 0.5 };
}

void USettingsManager::SaveAudioSettings()
{
	UADSettingsSaveGame* SaveGameInstance = Cast<UADSettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UADSettingsSaveGame::StaticClass())
	);

	SaveGameInstance->AudioSettings = CachedAudioSettings;
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);

}

void USettingsManager::LoadAudioSettings()
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		if (UADSettingsSaveGame* Loaded = Cast<UADSettingsSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
		{
			CachedAudioSettings = Loaded->AudioSettings;
		}
	}

	ApplyAudioSettings(CachedAudioSettings);
}

void USettingsManager::ApplyAudioSettings(const FUserAudioSettings& InSettings)
{
	CachedAudioSettings = InSettings;

	if (UADGameInstance* GI = GetWorld()->GetGameInstance<UADGameInstance>())
	{
		if (USoundSubsystem* SoundSubsystem = GI->GetSubsystem<USoundSubsystem>())
		{
			SoundSubsystem->ChangeMasterVolume(InSettings.MasterVolume);
			SoundSubsystem->ChangeBGMVolume(InSettings.BGMVolume);
			SoundSubsystem->ChangeSFXVolume(InSettings.SFXVolume);
			SoundSubsystem->ChangeAmbientVolume(InSettings.AmbientVolume);
		}
	}
}

void USettingsManager::SaveKeySettings()
{
	UADSettingsSaveGame* SaveGameInstance = Cast<UADSettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UADSettingsSaveGame::StaticClass())
	);

	SaveGameInstance->KeyBindings = CachedKeyBindings;
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
}

void USettingsManager::LoadKeySettings(APlayerController* PC)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		if (UADSettingsSaveGame* Loaded = Cast<UADSettingsSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
		{
			CachedKeyBindings = Loaded->KeyBindings;
		}
		
	}

	InitializeDefaultKeyBindingsIfEmpty();
	ApplyKeySettings(CachedKeyBindings, PC);
}

void USettingsManager::ApplyKeySettings(const TArray<FKeyBinding>& InBindings, APlayerController* PC)
{
	CachedKeyBindings = InBindings;

	if (!BaseMappingContext || !PC) return;

	// ����
	RuntimeMappingContext = DuplicateObject<UInputMappingContext>(BaseMappingContext, this);

	// Input Subsystem Ȯ��
	if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSubsystem->ClearAllMappings();
			InputSubsystem->AddMappingContext(RuntimeMappingContext, 0);
		}
	}

	// ���ε� ����
	for (const FKeyBinding& Binding : InBindings)
	{
		const FName& ActionName = Binding.ActionName;
		const FKey& Key = Binding.AssignedKey;

		// InputAction ã�� (��: �̸� �������� StaticMap���� �����ϰų� ����ص� �迭)
		const UInputAction* FoundAction = nullptr; // TODO: ActionName �� UInputAction* ������ �����ϴ� ������ �ʿ�

		if (FoundAction)
		{
			RuntimeMappingContext->UnmapKey(FoundAction, Key); // ���� Ű ����
			RuntimeMappingContext->MapKey(FoundAction, Key);   // �� Ű ����
		}
	}



}

void USettingsManager::SaveAllSettings()
{
	UADSettingsSaveGame* SaveObj = Cast<UADSettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UADSettingsSaveGame::StaticClass())
	);

	SaveObj->AudioSettings = CachedAudioSettings;
	SaveObj->KeyBindings = CachedKeyBindings;

	UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, 0);
}

void USettingsManager::LoadAllSettings(APlayerController* PC)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		if (UADSettingsSaveGame* Loaded = Cast<UADSettingsSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
		{
			CachedAudioSettings = Loaded->AudioSettings;
			CachedKeyBindings = Loaded->KeyBindings;
		}
	}

	InitializeDefaultKeyBindingsIfEmpty(); // Ű ���ε� ����� �� ä���
	ApplyAllSettings(PC);
}

void USettingsManager::ApplyAllSettings(APlayerController* PC)
{
	ApplyAudioSettings(CachedAudioSettings);
	ApplyKeySettings(CachedKeyBindings, PC);
}

void USettingsManager::UpdateCachedKeyBinding(FName ActionName, FKey NewKey)
{
	for (FKeyBinding& Binding : CachedKeyBindings)
	{
		if (Binding.ActionName == ActionName)
		{
			Binding.AssignedKey = NewKey;
			return;
		}
	}

	CachedKeyBindings.Add(FKeyBinding{ ActionName, NewKey });

}

void USettingsManager::InitializeDefaultKeyBindingsIfEmpty()
{

}
