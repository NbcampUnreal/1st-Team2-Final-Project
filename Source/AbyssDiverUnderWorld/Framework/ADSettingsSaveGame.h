#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ADSettingsSaveGame.generated.h"


USTRUCT(BlueprintType)
struct FUserAudioSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MasterVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BGMVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SFXVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AmbientVolume = 0.5f;
};

USTRUCT(BlueprintType)
struct FKeyBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey AssignedKey;
};

/** Mouse 설정 저장 구조체 */
USTRUCT(BlueprintType)
struct FUserMouseSettings
{
	GENERATED_BODY()

	/** 마우스 감도 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mouse")
	float Sensitivity = 1.0f;

	/** Y축 반전 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mouse")
	bool bInvertY = false;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FUserAudioSettings AudioSettings;

	UPROPERTY(BlueprintReadWrite)
	TArray<FKeyBinding> KeyBindings;

	/** 마우스 설정 */
	UPROPERTY(BlueprintReadWrite)
	FUserMouseSettings MouseSettings;

private:

	static const FString SavedAudioSettingsSlotName;
};
