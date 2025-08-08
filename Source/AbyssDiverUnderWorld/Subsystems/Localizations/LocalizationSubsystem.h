#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Subsystems/Localizations/LocalizationConstants.h"

#include "LocalizationSubsystem.generated.h"

enum class ELanguage : uint8
{
	English,
	Korean,
	MAX
};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API ULocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region Methods

public:

	FText GetLocalizedText(const FName& TableKey, const FTextKey& TextKey) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FText GetLocalizedTextFromStringKey(const FName& TableKey, const FString& TextKey) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FText GetLocalizedTextFromFName(const FName& TableKey, const FName& TextKey) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FText GetLocalizedItemName(const FName& ItemName) const;

	// PIE 환경에서 사용할 경우 에디터 언어만 바뀌고 게임 내의 언어는 바뀌지 않는 듯.
	void SetLanguage(ELanguage LanguageType);

#pragma endregion

};
