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

	/** 저장된 모든 설정을 불러옴 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadAllSettings(bool bShouldApplySettings = false);

	/** 현재 설정을 저장 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveAllSettings();

	/** Audio Subsystem에 설정을 적용 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void ApplyAudioSettings(const FUserAudioSettings& InSettings);

	/** 키 설정을 PC에 적용 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Control")
	void ApplyKeySettings(const TArray<FKeyBinding>& InBindings, APlayerController* PC);

	/** 마우스 설정을 PC에 적용 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Control")
	void ApplyMouseSettings(const FUserMouseSettings& NewMouseSettings, APlayerController* PC);

	/** 현재 설정값을 게임에 적용 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyCurrentSettings(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category="Settings|Key")
	void UpdateCachedKeyBinding(FName ActionName, FKey NewKey);

	UFUNCTION(BlueprintCallable, Category = "Settings|Control")
	void InitializeActionMap(const TMap<FName, UInputAction*>& InMap);

protected:
	
	//Audio Settings
	bool PopulateAudioSettings(UADSettingsSaveGame* SaveGame);

	//Control Settings
	UFUNCTION(BlueprintCallable, Category = "Settings|Control")
	void SaveKeySettings();

	/** 마우스 설정 저장 */
	bool PopulateMouseSettings(UADSettingsSaveGame* SaveGame);

private:

	void InitializeDefaultKeyBindingsIfEmpty();

protected:
	UPROPERTY(BlueprintReadOnly)
	FUserAudioSettings CachedAudioSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> BaseMappingContext;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> RuntimeMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<FName, UInputAction*> ActionMap;

	UPROPERTY(BlueprintReadOnly)
	TArray<FKeyBinding> CachedKeyBindings;

	/** 마우스 설정 캐시 */
	UPROPERTY(BlueprintReadOnly)
	FUserMouseSettings MouseSettings;

	static const FString SlotName;

public:
	UFUNCTION(BlueprintPure, Category="Settings|Audio")
	FUserAudioSettings GetCachedAudioSettings() const { return CachedAudioSettings; }

	UFUNCTION(BlueprintPure, Category = "Settings|Control")
	TArray<FKeyBinding> GetCachedKeyBindings() const { return CachedKeyBindings; }

	/** 현재 마우스 설정을 반환 */
	UFUNCTION(BlueprintPure, Category = "Settings|Control")
	FUserMouseSettings GetCachedMouseSettings() const { return MouseSettings; }
	
};
