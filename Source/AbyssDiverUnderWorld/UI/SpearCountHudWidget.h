#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpearCountHudWidget.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API USpearCountHudWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 생성자
    USpearCountHudWidget(const FObjectInitializer& ObjectInitializer);

protected:
    // 생명주기 함수
    virtual void NativeConstruct() override;

#pragma region Method
public:
    // 작살 수 세팅
    void SetSpearCount(int32 Current, int32 Total);

    // 산소 게이지 세팅
    void SetOxygenPercent(float InPercent);

protected:
private:
#pragma endregion

#pragma region Variable
public:
    // 텍스트 바인딩
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SpearNum;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TotalSpear;

    // 산소 ProgressBar
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* OxygenBar;

protected:
private:
#pragma endregion

#pragma region Getter, Setter
public:
    FORCEINLINE int32 GetCurrentSpear() const { return CurrentSpear; }
    FORCEINLINE int32 GetTotalSpear() const { return TotalSpearCount; }

    void SetCurrentSpear(int32 InValue);
    void SetTotalSpear(int32 InValue);

protected:
private:
    int32 CurrentSpear = 0;
    int32 TotalSpearCount = 0;
#pragma endregion
};
