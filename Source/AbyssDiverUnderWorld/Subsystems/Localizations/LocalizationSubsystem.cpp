#include "Subsystems/Localizations/LocalizationSubsystem.h"

#include "AbyssDiverUnderWorld.h"

void ULocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

FText ULocalizationSubsystem::GetLocalizedText(const FName& TableKey, const FTextKey& TextKey) const
{
	return FText::FromStringTable(TableKey, TextKey);
}

FText ULocalizationSubsystem::GetLocalizedTextFromStringKey(const FName& TableKey, const FString& TextKey) const
{
	return GetLocalizedText(TableKey, TextKey);
}

FText ULocalizationSubsystem::GetLocalizedTextFromFName(const FName& TableKey, const FName& TextKey) const
{
	return GetLocalizedTextFromStringKey(TableKey, TextKey.ToString());
}

FText ULocalizationSubsystem::GetLocalizedItemName(const FName& ItemName) const
{
	return GetLocalizedTextFromStringKey(ST_ItemName::TableKey, ItemName.ToString());
}

void ULocalizationSubsystem::SetLanguage(ELanguage LanguageType)
{
	switch (LanguageType)
	{
	case ELanguage::English:
		FInternationalization::Get().SetCurrentLanguage("en");
		break;
	case ELanguage::Korean:
		FInternationalization::Get().SetCurrentLanguage("ko");
		break;
	case ELanguage::MAX:
		check(false);
		return;
	default:
		check(false);
		return;
	}
}
