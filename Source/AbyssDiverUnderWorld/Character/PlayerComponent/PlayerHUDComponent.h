#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/PlayerStatusWidget.h"
#include "PlayerHUDComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UPlayerHUDComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerHUDComponent();

protected:
    virtual void BeginPlay() override;

#pragma region Method
public:
    UFUNCTION(Client, Reliable)
    void C_ShowResultScreen();
    void C_ShowResultScreen_Implementation();

    void SetVisibility(bool NewVisible) const;
    void SetResultScreenVisible(const bool bShouldVisible) const;
    void UpdateResultScreen(int32 PlayerIndexBased_1, const struct FResultScreenParams& Params);

    FORCEINLINE TObjectPtr<UPlayerStatusWidget> GetPlayerStatusWidget() const { return PlayerStatusWidget; }

    UFUNCTION()
    void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

    UFUNCTION()
    void UpdateOxygenHUD(float CurrentOxygenLevel, float MaxOxygenLevel);

    UFUNCTION()
    void UpdateStaminaHUD(float CurrentStamina, float MaxStamina);

    UFUNCTION()
    void UpdateHealthHUD(int32 CurrentHealth, int32 MaxHealth);

    UFUNCTION()
    void UpdateSpearCount(const int32& CurrentSpear, const int32& TotalSpear);

private:
#pragma endregion

#pragma region Variable
private:
    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UPlayerHUDWidget> HudWidgetClass;

    UPROPERTY()
    TObjectPtr<class UPlayerHUDWidget> HudWidget;

    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UResultScreen> ResultScreenWidgetClass;

    UPROPERTY()
    TObjectPtr<class UResultScreen> ResultScreenWidget;

    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UPlayerStatusWidget> PlayerStatusWidgetClass;

    UPROPERTY()
    TObjectPtr<UPlayerStatusWidget> PlayerStatusWidget;
#pragma endregion
};