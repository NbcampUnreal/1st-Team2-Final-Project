#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/PlayerStatusWidget.h"
#include "PlayerHUDComponent.generated.h"

enum class EMissionType : uint8;

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

    UFUNCTION(NetMulticast, Reliable)
    void M_SetSpearUIVisibility(bool bVisible);
    void M_SetSpearUIVisibility_Implementation(bool bVisible);

    UFUNCTION(NetMulticast, Reliable)
    void M_UpdateSpearCount(const int32& CurrentSpear, const int32& TotalSpear);
    void M_UpdateSpearCount_Implementation(const int32& CurrentSpear, const int32& TotalSpear);

    /*UFUNCTION(NetMulticast, Reliable)
    void M_SetSpearGunTypeImage(int8 TypeNum);
    void M_SetSpearGunTypeImage_Implementation(int8 TypeNum);*/
    UFUNCTION(Client, Reliable)
    void C_SetSpearGunTypeImage(int8 TypeNum);
    void C_SetSpearGunTypeImage_Implementation(int8 TypeNum);

    /** Test HUD 보이기 설정 */
    void SetTestHUDVisibility(bool NewVisible) const;
    
    /** Test HUD 보이기 토글 */
    void ToggleTestHUD() const;
    
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

    void UpdateMissionsOnHUD(EMissionType MissionType, uint8 MissionIndex, int32 CurrentProgress);

    void PlayNextPhaseAnim(int32 NextPhaseNumber);
    void SetCurrentPhaseOverlayVisible(bool bShouldVisible);

private:
#pragma endregion

#pragma region Variable
private:
    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UPlayerHUDWidget> HudWidgetClass;

    UPROPERTY()
    TObjectPtr<class UPlayerHUDWidget> HudWidget;

    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UCrosshairWidget> CrosshairWidgetClass;

    UPROPERTY()
    TObjectPtr<class UCrosshairWidget> CrosshairWidget;

    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UResultScreen> ResultScreenWidgetClass;

    UPROPERTY()
    TObjectPtr<class UResultScreen> ResultScreenWidget;

    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UPlayerStatusWidget> PlayerStatusWidgetClass;

    UPROPERTY(BlueprintReadOnly, meta =(AllowPrivateAccess = true))
    TObjectPtr<UPlayerStatusWidget> PlayerStatusWidget;

    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UMissionsOnHUDWidget> MissionsOnHUDWidgetClass;

    UPROPERTY()
    TObjectPtr<class UMissionsOnHUDWidget> MissionsOnHUDWidget;
    
#pragma endregion

#pragma region Getter Setter

public:

    bool IsTestHUDVisible() const;

    UMissionsOnHUDWidget* GetMissionsOnHudWidget() const;
    
#pragma endregion
};