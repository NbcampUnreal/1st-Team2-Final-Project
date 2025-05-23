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
    UPlayerStatusWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;

#pragma region Method
public:
    void SetSpearCount(int32 Current, int32 Total);
    void SetOxygenPercent(float InPercent);
    void SetHealthPercent(float InPercent);      // ✅ 분절형 체력 구현용
    void SetStaminaPercent(float InPercent);
#pragma endregion

#pragma region Getter/Setter
public:
    FORCEINLINE int32 GetCurrentSpear() const { return CurrentSpear; }
    FORCEINLINE int32 GetTotalSpear() const { return TotalSpearCount; }
    void SetCurrentSpear(int32 InValue);
    void SetTotalSpear(int32 InValue);
#pragma endregion

#pragma region Variable
protected:
    UPROPERTY()
    int32 CurrentSpear = 0;

    UPROPERTY()
    int32 TotalSpearCount = 0;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> SpearNum;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TotalSpear;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> OxygenBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HealthBar;


    // ✅ 스태미나
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> StaminaBar;
#pragma endregion

private:
    UPROPERTY()
    TArray<TObjectPtr<UImage>> HealthSegments;


};
