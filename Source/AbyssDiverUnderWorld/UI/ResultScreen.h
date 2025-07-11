﻿#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ResultScreen.generated.h"

class UTextBlock;

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

	FResultScreenParams(const FString& InPlayerNicName, int32 InContributionScore, int32 InOreScore, EAliveInfo InAliveInfo)
	{
		PlayerNickName = InPlayerNicName;
		ContributionScore = InContributionScore;
		OreScore = InOreScore;
		AliveInfo = InAliveInfo;
	}

	FString PlayerNickName;
	int32 ContributionScore;
	int32 OreScore;
	EAliveInfo AliveInfo;
};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UResultScreen : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;

#pragma region Methods

public:

	// Player Index는 1부터 시작
	void Update(int32 PlayerIndex, const FResultScreenParams& Params);

	// Player Index는 1부터 시작
	void ChangePlayerNickNameText(int32 PlayerIndex, const FString& NewText);

	// Player Index는 1부터 시작
	void ChangePlayerAliveText(int32 PlayerIndex, EAliveInfo AliveInfo);
	void ChangePlayerAliveText(int32 PlayerIndex, const FString& NewText);

	// Player Index는 1부터 시작
	void ChangePlayerContributionText(int32 PlayerIndex, int32 ContributionScore);
	void ChangePlayerContributionText(int32 PlayerIndex, const FString& NewText);

	// Player Index는 1부터 시작
	void ChangePlayerOreText(int32 PlayerIndex, int32 OreScore);
	void ChangePlayerOreText(int32 PlayerIndex, const FString& NewText);

	// Player Index는 1부터 시작
	void ChangeTeamMoneyText(int32 NewTeamMoney);
	void ChangeTeamMoneyText(const FString& NewText);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> PlayerNickNameTextArray;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> PlayerAliveTextArray;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> PlayerContributionTextArray;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> PlayerOreTextArray;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamMoneyText;

#pragma endregion

	
};
