#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

#include "PlayerStatusWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UOverlay;
enum class ESpearGunType : uint8;
class UWarningWidget;

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

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Compass")
	void SetCompassObjectWidgetVisible(bool bShouldVisible);

	// 일반 함수
	void SetSpearCount(int32 Current, int32 Total);
	void SetOxygenPercent(float InPercent);
	void SetHealthPercent(float InPercent);
	void SetStaminaPercent(float InPercent);
	void SetDroneCurrentText(int32 Current);
	void SetDroneTargetText(int32 Target);
	void SetMoneyProgressBar(float InPercent);

	void PlayNextPhaseAnim(int32 NextPhaseNumber);

	void SetCurrentPhaseText(const FString& PhaseText);
	void SetNextPhaseText(const FString& PhaseText);

	void SetCurrentPhaseOverlayVisible(bool bShouldVisible);

	void SetSpearGunTypeImage(int8 TypeNum);

private:

	UFUNCTION()
	void OnNextPhaseAnimFinished();

	bool TryPlayAnim(UWidgetAnimation* Anim);

#pragma endregion

#pragma region Variable

protected:

	// 작살 수치
	UPROPERTY()
	int32 CurrentSpear = 0;

	UPROPERTY()
	int32 TotalSpearCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UTexture2D>> SpearGunTypeImages;

	// 바인딩된 텍스트 및 프로그레스바 위젯
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpearNum;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalSpear;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> OxygenBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> SpearPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Compass")
	TObjectPtr<AActor> CompassTargetObject;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentMoneyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TargetMoneyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> CurrentPhaseOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentPhaseText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> NextPhaseOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NextPhaseText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> NextPhaseAnim;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HealthScreenEffect;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> MoneyProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SpearGunTypeImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWarningWidget> OxygenWarningWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWarningWidget> PhaseWarningWidget;

	UPROPERTY()
	TObjectPtr <UMaterialInstanceDynamic> DynamicMaterial;

	UPROPERTY()
	TObjectPtr <UMaterialInterface> LoadedMaterial;


private:

	UPROPERTY()
	TArray<TObjectPtr<UImage>> HealthSegments;

	static const FName OnNextPhaseAnimFinishedName;

	int32 CachedNextPhaseNumber = 0;

	static const int32 MaxPhaseNumber;

	float Period = 0.0f;

#pragma endregion

#pragma region Getter, Setter

public:

	FORCEINLINE int32 GetCurrentSpear() const { return CurrentSpear; }
	FORCEINLINE int32 GetTotalSpear() const { return TotalSpearCount; }
	int8 GetNextPhaseAnimEndTime() const;
	void SetCurrentSpear(int32 InValue);
	void SetTotalSpear(int32 InValue);
	void SetSpearVisibility(bool bVisible);
	void SetCompassObject(AActor* NewTargetObject);

#pragma endregion
};
