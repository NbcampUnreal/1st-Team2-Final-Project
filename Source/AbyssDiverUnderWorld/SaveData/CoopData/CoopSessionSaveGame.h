#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "Container/FStructContainer.h"

#include "CoopSessionSaveGame.generated.h"

struct FItemData;

USTRUCT()
struct FPlayerProgressData
{
	GENERATED_BODY()

public:

	FPlayerProgressData()
	{

	}

	FPlayerProgressData(const FString& InPlayerNetId, const TArray<FItemData>& InItemDataList,const  TArray<uint8>& InUpgradeGradeMap)
	{
		PlayerNetId = InPlayerNetId;
		ItemDataList = InItemDataList;
		UpgradeGradeMap = InUpgradeGradeMap;
	}

	UPROPERTY()
	FString PlayerNetId;

	// 인벤토리 아이템 정보
	UPROPERTY()
	TArray<FItemData> ItemDataList;

	// 업그레이드 정보
	UPROPERTY()
	TArray<uint8> UpgradeGradeMap;

};

USTRUCT()
struct FCoopSessionSaveData
{
	GENERATED_BODY()

public:

	FCoopSessionSaveData()
	{

	}

	FCoopSessionSaveData(const TArray<FPlayerProgressData>& InPlayerProgressDataList, int32 InClearCount, int32 InTotalMoney, const FString& InSaveDataName)
	{
		PlayerProgressDataList = InPlayerProgressDataList;
		ClearCount = InClearCount;
		TotalMoney = InTotalMoney;
		SaveDataName = InSaveDataName;
	}

	UPROPERTY()
	TArray<FPlayerProgressData> PlayerProgressDataList;

	UPROPERTY()
	int32 ClearCount = 0;

	UPROPERTY()
	int32 TotalMoney = 0;

	UPROPERTY()
	FString SaveDataName = "";
};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UCoopSessionSaveGame : public USaveGame
{
	GENERATED_BODY()

#pragma region Variables

private:

	UPROPERTY()
	FCoopSessionSaveData SessionSaveData;

#pragma endregion

#pragma region Getter / Setter
	
public:

	const FCoopSessionSaveData& GetSaveData() const;
	void SetSaveData(const FCoopSessionSaveData& InSessionSaveData);

#pragma endregion
};
