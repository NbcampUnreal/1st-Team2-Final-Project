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

	// 복제
	RuntimeMappingContext = DuplicateObject<UInputMappingContext>(BaseMappingContext, this);

	// Input Subsystem 확보
	if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSubsystem->ClearAllMappings();
			InputSubsystem->AddMappingContext(RuntimeMappingContext, 0);
		}
	}

	// 바인딩 변경
	for (const FKeyBinding& Binding : InBindings)
	{
		const FName& ActionName = Binding.ActionName;
		const FKey& Key = Binding.AssignedKey;

		// InputAction 찾기 (예: 이름 기준으로 StaticMap에서 참조하거나 등록해둔 배열)
		const UInputAction* FoundAction = ActionMap.FindRef(ActionName);

		if (FoundAction)
		{
			for (const FEnhancedActionKeyMapping& Mapping : RuntimeMappingContext->GetMappings())
			{
				if (Mapping.Action == FoundAction)
				{
					RuntimeMappingContext->UnmapKey(FoundAction, Mapping.Key);
				}
			}

			// 2. 새 키 매핑
			RuntimeMappingContext->MapKey(FoundAction, Key);
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

	InitializeDefaultKeyBindingsIfEmpty(); // 키 바인딩 비었을 때 채우기
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

void USettingsManager::InitializeActionMap(const TMap<FName, TObjectPtr<UInputAction>>& InMap)
{
	ActionMap = InMap;
}

void USettingsManager::InitializeDefaultKeyBindingsIfEmpty()
{
	if (!CachedKeyBindings.IsEmpty())
		return;

	CachedKeyBindings.Add({ "Ascend", EKeys::SpaceBar });            // 수직 상승
	CachedKeyBindings.Add({ "Descend", EKeys::C });                  // 수직 하강
	CachedKeyBindings.Add({ "Sprint", EKeys::LeftShift });           // 빠른 이동
	CachedKeyBindings.Add({ "Fire", EKeys::LeftMouseButton });       // 발사
	CachedKeyBindings.Add({ "Aim", EKeys::RightMouseButton });       // 조준
	CachedKeyBindings.Add({ "Interact", EKeys::E });                 // 상호작용
	CachedKeyBindings.Add({ "SlotScroll", EKeys::MouseWheelAxis });  // 마우스 휠 (스크롤)
	CachedKeyBindings.Add({ "Light", EKeys::F });                    // 라이트
	CachedKeyBindings.Add({ "Inventory", EKeys::Tab });              // 인벤토리
	CachedKeyBindings.Add({ "Radar", EKeys::Q });                    // 레이더
	CachedKeyBindings.Add({ "Reload", EKeys::R });                   // 재장전
	CachedKeyBindings.Add({ "Jump", EKeys::SpaceBar });              // 점프

	// 슬롯 1~5
	CachedKeyBindings.Add({ "Slot1", EKeys::One });
	CachedKeyBindings.Add({ "Slot2", EKeys::Two });
	CachedKeyBindings.Add({ "Slot3", EKeys::Three });
	CachedKeyBindings.Add({ "Slot4", EKeys::Four });
	CachedKeyBindings.Add({ "Slot5", EKeys::Five });
}
