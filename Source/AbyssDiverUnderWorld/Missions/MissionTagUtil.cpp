#include "Missions/MissionTagUtil.h"
#include "GameplayTagsManager.h"

namespace
{
	template<typename TEnum>
	FString GetEnumTail(const TEnum Value)
	{
		if (const UEnum* E = StaticEnum<TEnum>())
		{
#if WITH_EDITOR
			const FText Disp = E->GetDisplayNameTextByValue((int64)Value);
			if (!Disp.IsEmpty())
			{
				return Disp.ToString().Replace(TEXT(" "), TEXT("")); // 공백 제거
			}
#endif
			return E->GetNameStringByValue((int64)Value);
		}
		return TEXT("None");
	}

	static FGameplayTag RequestUnderRoot(const TCHAR* Root, const FString& Tail)
	{
		const FString Path = FString::Printf(TEXT("%s.%s"), Root, *Tail);
		const FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName(*Path), /*ErrorIfNotFound*/ false);
		return Tag;
	}
}

// ---- 내부 헬퍼 ----
template<typename TEnum>
FString UMissionTagUtil::EnumToTailString(TEnum Value)
{
	return GetEnumTail<TEnum>(Value);
}

FGameplayTag UMissionTagUtil::RequestTagChecked(const TCHAR* Root, const FString& Tail)
{
	return RequestUnderRoot(Root, Tail);
}

// ---- Unit ----
FGameplayTag UMissionTagUtil::ToUnitIdTag(EUnitId UnitId)
{
	return RequestTagChecked(Root_UnitId(), EnumToTailString(UnitId));
}
FGameplayTag UMissionTagUtil::ToUnitTypeTagByTail(FName CategoryTail)
{
	return RequestTagChecked(Root_UnitType(), CategoryTail.ToString());
}
bool UMissionTagUtil::IsUnitIdTag(const FGameplayTag& Tag)
{
	return Tag.IsValid() && Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Unit.id")));
}
bool UMissionTagUtil::IsUnitTypeTag(const FGameplayTag& Tag)
{
	return Tag.IsValid() && Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Unit.Type")));
}

// ---- Item ----
FGameplayTag UMissionTagUtil::ToItemIdTag(uint8 ItemId)
{
	return RequestTagChecked(Root_ItemId(), EnumToTailString(ItemId));
}
FGameplayTag UMissionTagUtil::ToItemTypeTagByTail(FName CategoryTail)
{
	return RequestTagChecked(Root_ItemType(), CategoryTail.ToString());
}
bool UMissionTagUtil::IsItemIdTag(const FGameplayTag& Tag)
{
	return Tag.IsValid() && Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Item.id")));
}
bool UMissionTagUtil::IsItemTypeTag(const FGameplayTag& Tag)
{
	return Tag.IsValid() && Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Item.Type")));
}

// ---- Debug ----
FString UMissionTagUtil::GetTailFromTag(const FGameplayTag& Tag)
{
	if (!Tag.IsValid()) return TEXT("");
	FString Head, Tail;
	return Tag.ToString().Split(TEXT("."), &Head, &Tail, ESearchCase::IgnoreCase, ESearchDir::FromEnd) ? Tail : Tag.ToString();
}