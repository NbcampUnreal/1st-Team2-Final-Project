#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ADSettingsSaveGame.h"
#include "InputMappingContext.h"
#include "SettingsManager.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API USettingsManager : public UObject
{
	GENERATED_BODY()

public:
	USettingsManager();

	
	
public:
	//Audio Settgins
	UFUNCTION(BlueprintCallable, Category="Settings|Audio")
	void SaveAudioSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void LoadAudioSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void ApplyAudioSettings(const FUserAudioSettings& InSettings);
	
	//Control Settings
	UFUNCTION(BlueprintCallable, Category = "Settings|Control")
	void SaveKeySettings();

	UFUNCTION(BlueprintCallable, Category = "Settings|Control")
	void LoadKeySettings(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Settings|Control")
	void ApplyKeySettings(const TArray<FKeyBinding>& InBindings, APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveAllSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadAllSettings(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAllSettings(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category="Settings|Key")
	void UpdateCachedKeyBinding(FName ActionName, FKey NewKey);

	UFUNCTION(BlueprintCallable, Category = "Settings|Control")
	void InitializeActionMap(const TMap<FName, UInputAction*>& InMap);
private:



	void InitializeDefaultKeyBindingsIfEmpty();

protected:
	UPROPERTY(BlueprintReadOnly)
	FUserAudioSettings CachedAudioSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> BaseMappingContext;

	TObjectPtr<UInputMappingContext> RuntimeMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<FName, UInputAction*> ActionMap;

	UPROPERTY(BlueprintReadOnly)
	TArray<FKeyBinding> CachedKeyBindings;

	const FString SlotName = TEXT("SettingsSlot");

public:
	UFUNCTION(BlueprintPure, Category="Settings|Audio")
	FUserAudioSettings GetCachedAudioSettings() const { return CachedAudioSettings; }

	UFUNCTION(BlueprintPure, Category = "Settings|Control")
	TArray<FKeyBinding> GetCachedKeyBindings() const { return CachedKeyBindings; }

};
