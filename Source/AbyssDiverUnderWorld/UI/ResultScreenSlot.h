// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultScreenSlot.generated.h"

enum class EAliveInfo
{
	Alive,
	Abandoned,
	Dead,
	MAX UMETA(Hidden)
};

struct FResultScreenParams
{
	FResultScreenParams()
	{

	}

	FResultScreenParams(const FString& InPlayerNicName, EAliveInfo InAliveInfo, int32 InContributionScore, int32 InBattleScore, int32 InSupportScore)
	{
		PlayerNickName = InPlayerNicName;
		AliveInfo = InAliveInfo;
		CollectionScore = InContributionScore;
		BattleScore = InBattleScore;
		SupportScore = InSupportScore;
	}

	FString PlayerNickName;
	EAliveInfo AliveInfo;
	int32 CollectionScore;
	int32 BattleScore;
	int32 SupportScore;

	// 정렬용 변수
	float NormalizedCollectScore = 0.0f;
	float NormalizedCombatScore = 0.0f;
	float NormalizedSupportScore = 0.0f;
	float MVPScore = 0.0f;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UResultScreenSlot : public UUserWidget
{
	GENERATED_BODY()

#pragma region Methods
public:
	void SetSlotInfo(const FResultScreenParams& Params);

	void ChangePlayerNickNameText(const FString& NewText);

	void ChangePlayerAliveText(EAliveInfo AliveInfo);

	void ChangePlayerCollectionText(const FString& NewText);

	void ChangePlayerBattleText(const FString& NewText);

	void ChangePlayerSupportText(const FString& NewText);


#pragma endregion Methods	
	
#pragma region Variables

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerNickNameText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerAliveText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerCollectionText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerBattleText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerSupportText;

#pragma endregion Variables
};
