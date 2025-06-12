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

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FUserAudioSettings AudioSettings;

	UPROPERTY(BlueprintReadWrite)
	TArray<FKeyBinding> KeyBindings;

};
