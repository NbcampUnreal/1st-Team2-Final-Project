#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ResultScreen.generated.h"

class UTextBlock;
class UResultScreenSlot;
struct FResultScreenParams;
enum class EAliveInfo;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UResultScreen : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;

#pragma region Methods

public:

	void Update(int32 PlayerIndex, const FResultScreenParams& Params);

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

	UPROPERTY()
	TArray<TObjectPtr<UResultScreenSlot>> PlayerResultInfosArray;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamMoneyText;

#pragma endregion

	
};
