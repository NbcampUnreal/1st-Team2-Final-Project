#include "Framework/SettingsManager.h"

#include "AbyssDiverUnderWorld.h"
#include "ADGameInstance.h"
#include "ADPlayerController.h"
#include "Subsystems/SoundSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"

const FString USettingsManager::SlotName = TEXT("SettingsSlot");

USettingsManager::USettingsManager()
{
	CachedAudioSettings = { 0.5, 0.5, 0.5, 0.5 };
}

bool USettingsManager::PopulateAudioSettings(UADSettingsSaveGame* OutSaveGame)
{
	if (OutSaveGame == nullptr)
	{
		return false;
	}
	
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

	CachedAudioSettings.AmbientVolume = SoundSubsystem->GetAmbientVolume();
	CachedAudioSettings.BGMVolume = SoundSubsystem->GetBGMVolume();
	CachedAudioSettings.MasterVolume = SoundSubsystem->GetMasterVolume();
	CachedAudioSettings.SFXVolume = SoundSubsystem->GetSFXVolume();

	OutSaveGame->AudioSettings = CachedAudioSettings;
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

bool USettingsManager::PopulateMouseSettings(UADSettingsSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return false;
	}

	SaveGame->MouseSettings = MouseSettings;
	return true;
}

void USettingsManager::ApplyKeySettings(const TArray<FKeyBinding>& InBindings, APlayerController* PC)
{
	CachedKeyBindings = InBindings;

	if (!BaseMappingContext || !PC) return;

	// Context 복제
	RuntimeMappingContext = DuplicateObject<UInputMappingContext>(BaseMappingContext, this);
	// 기존 매핑 제거 → 정확한 키 매핑 쌍 제거
	for (auto& Pair : ActionMap)
	{
		if (Pair.Value)
		{
			RuntimeMappingContext->UnmapAllKeysFromAction(Pair.Value); // 전체 키 제거
		}
	}

	// 새 매핑 적용
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
			InputSubsystem->ClearAllMappings(); // 모든 기존 맵핑 제거
			InputSubsystem->AddMappingContext(RuntimeMappingContext, 0); // 새 컨텍스트 삽입
		}
	}

}

void USettingsManager::ApplyMouseSettings(const FUserMouseSettings& NewMouseSettings, APlayerController* PC)
{
	MouseSettings = NewMouseSettings;

	if (AADPlayerController* ADPlayerController =  Cast<AADPlayerController>(PC))
	{
		ADPlayerController->SetLookSensitivity(NewMouseSettings.Sensitivity, NewMouseSettings.Sensitivity);
	}
}

void USettingsManager::SaveAllSettings()
{
	UADSettingsSaveGame* SaveGameObject = Cast<UADSettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UADSettingsSaveGame::StaticClass())
	);

	PopulateAudioSettings(SaveGameObject);
	// SaveGameObject->KeyBindings = CachedKeyBindings;
	PopulateMouseSettings(SaveGameObject);

	UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);
}

void USettingsManager::LoadAllSettings(bool bShouldApplySettings)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		if (UADSettingsSaveGame* Loaded = Cast<UADSettingsSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
		{
			CachedAudioSettings = Loaded->AudioSettings;
			CachedKeyBindings = Loaded->KeyBindings;
			MouseSettings = Loaded->MouseSettings;
		}
	}

	InitializeDefaultKeyBindingsIfEmpty(); // 키 바인딩 비었을 때 채우기

	if (bShouldApplySettings)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		ApplyCurrentSettings(PC);
	}
}

void USettingsManager::ApplyCurrentSettings(APlayerController* PC)
{
	ApplyAudioSettings(CachedAudioSettings);
	// ApplyKeySettings(CachedKeyBindings, PC);
	ApplyMouseSettings(MouseSettings, PC);
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
