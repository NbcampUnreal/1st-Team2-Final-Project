#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
    // 작살 수 세팅
    void SetSpearCount(int32 Current, int32 Total);

    // 산소 게이지 세팅
    void SetOxygenPercent(float InPercent);
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
    // 현재 작살 수
    UPROPERTY()
    int32 CurrentSpear = 0;

    // 총 작살 수
    UPROPERTY()
    int32 TotalSpearCount = 0;

    // 작살 수 텍스트
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> SpearNum;

    // 총 작살 수 텍스트
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TotalSpear;

    // 산소 게이지 바
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> OxygenBar;
#pragma endregion
};
