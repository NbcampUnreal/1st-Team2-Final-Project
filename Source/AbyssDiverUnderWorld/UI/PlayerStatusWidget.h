#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "PlayerStatusWidget.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 생성자
	UPlayerStatusWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// 생명주기 함수
	virtual void NativeConstruct() override;

#pragma region Method
public:
	// 일반 함수
	void SetSpearCount(int32 Current, int32 Total);
	void SetOxygenPercent(float InPercent);
	void SetHealthPercent(float InPercent);
	void SetStaminaPercent(float InPercent);
#pragma endregion

#pragma region Variable
protected:
	// 작살 수치
	UPROPERTY()
	int32 CurrentSpear = 0;

	UPROPERTY()
	int32 TotalSpearCount = 0;

	// 바인딩된 텍스트 및 프로그레스바 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpearNum;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalSpear;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> OxygenBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> SpearPanel;

private:
	UPROPERTY()
	TArray<TObjectPtr<UImage>> HealthSegments;
#pragma endregion

#pragma region Getter, Setter
public:
	// getter / setter
	FORCEINLINE int32 GetCurrentSpear() const { return CurrentSpear; }
	FORCEINLINE int32 GetTotalSpear() const { return TotalSpearCount; }
	void SetCurrentSpear(int32 InValue);
	void SetTotalSpear(int32 InValue);
	void SetSpearVisibility(bool bVisible);
#pragma endregion
};
