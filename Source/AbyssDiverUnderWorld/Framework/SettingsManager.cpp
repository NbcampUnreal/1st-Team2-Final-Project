#include "Framework/SettingsManager.h"

#include "AbyssDiverUnderWorld.h"
#include "ADGameInstance.h"
#include "Subsystems/SoundSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"

const FString USettingsManager::SlotName = TEXT("SettingsSlot");

USettingsManager::USettingsManager()
{
	CachedAudioSettings = { 0.5, 0.5, 0.5, 0.5 };
}

bool USettingsManager::SaveAudioSettings()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		LOGV(Error, TEXT("Fail to Save Audio settings"));
		return false;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(World);
	if (GI == nullptr)
	{
		LOGV(Error, TEXT("Fail to Save Audio settings"));
		return false;
	}

	USoundSubsystem* SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	if (SoundSubsystem == nullptr)
	{
		LOGV(Error, TEXT("Fail to Save Audio settings"));
		return false;
	}

	UADSettingsSaveGame* SaveGameInstance = Cast<UADSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UADSettingsSaveGame::StaticClass()));
	if (SaveGameInstance == nullptr)
	{
		LOGV(Error, TEXT("Fail to Save Audio settings"));
		return false;
	}

	CachedAudioSettings.AmbientVolume = SoundSubsystem->GetAmbientVolume();
	CachedAudioSettings.BGMVolume = SoundSubsystem->GetBGMVolume();
	CachedAudioSettings.MasterVolume = SoundSubsystem->GetMasterVolume();
	CachedAudioSettings.SFXVolume = SoundSubsystem->GetSFXVolume();

	SaveGameInstance->AudioSettings = CachedAudioSettings;
	return UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
}

bool USettingsManager::LoadAudioSettings()
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0) == false)
	{
		LOGV(Error, TEXT("Fail to Load AudioSettings"));
		return false;
	}

	UADSettingsSaveGame* Loaded = Cast<UADSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (Loaded == nullptr)
	{
		LOGV(Error, TEXT("Fail to Load AudioSettings"));
		return false;
	}

	CachedAudioSettings = Loaded->AudioSettings;
	ApplyAudioSettings(CachedAudioSettings);

	return true;
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

	// Context ����
	RuntimeMappingContext = DuplicateObject<UInputMappingContext>(BaseMappingContext, this);
	// ���� ���� ���� �� ��Ȯ�� Ű ���� �� ����
	for (auto& Pair : ActionMap)
	{
		if (Pair.Value)
		{
			RuntimeMappingContext->UnmapAllKeysFromAction(Pair.Value); // ��ü Ű ����
		}
	}

	// �� ���� ����
	for (const FKeyBinding& Binding : InBindings)
	{
		if (const UInputAction* FoundAction = ActionMap.FindRef(Binding.ActionName))
		{
			RuntimeMappingContext->MapKey(FoundAction, Binding.AssignedKey);
		}
	}

	if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSubsystem->ClearAllMappings(); // ��� ���� ���� ����
			InputSubsystem->AddMappingContext(RuntimeMappingContext, 0); // �� ���ؽ�Ʈ ����
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

void USettingsManager::InitializeActionMap(const TMap<FName, UInputAction*>& InMap)
{
	ActionMap = InMap;
}

void USettingsManager::InitializeDefaultKeyBindingsIfEmpty()
{
	if (!CachedKeyBindings.IsEmpty())
		return;

	CachedKeyBindings.Add({ "Ascend", EKeys::SpaceBar });            // ���� ���
	CachedKeyBindings.Add({ "Descend", EKeys::C });                  // ���� �ϰ�
	CachedKeyBindings.Add({ "Sprint", EKeys::LeftShift });           // ���� �̵�
	CachedKeyBindings.Add({ "Fire", EKeys::LeftMouseButton });       // �߻�
	CachedKeyBindings.Add({ "Aim", EKeys::RightMouseButton });       // ����
	CachedKeyBindings.Add({ "Interact", EKeys::E });                 // ��ȣ�ۿ�
	CachedKeyBindings.Add({ "SlotScroll", EKeys::MouseWheelAxis });  // ���콺 �� (��ũ��)
	CachedKeyBindings.Add({ "Light", EKeys::F });                    // ����Ʈ
	CachedKeyBindings.Add({ "Inventory", EKeys::Tab });              // �κ��丮
	CachedKeyBindings.Add({ "Radar", EKeys::Q });                    // ���̴�
	CachedKeyBindings.Add({ "Reload", EKeys::R });                   // ������
	CachedKeyBindings.Add({ "Jump", EKeys::SpaceBar });              // ����

	// ���� 1~5
	CachedKeyBindings.Add({ "Slot1", EKeys::One });
	CachedKeyBindings.Add({ "Slot2", EKeys::Two });
	CachedKeyBindings.Add({ "Slot3", EKeys::Three });
	CachedKeyBindings.Add({ "Slot4", EKeys::Four });
	CachedKeyBindings.Add({ "Slot5", EKeys::Five });
}
