#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/PlayerStatusWidget.h"
#include "PlayerHUDComponent.generated.h"

enum class EMissionType : uint8;
class USoundSubsystem;
class UDepthComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UPlayerHUDComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerHUDComponent();

protected:
    UFUNCTION()
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

    UFUNCTION()
    void OnShieldUseFailed();

    void UpdateMissionsOnHUD(EMissionType MissionType, uint8 MissionIndex, int32 CurrentProgress);

    void PlayNextPhaseAnim(int32 NextPhaseNumber);
    void SetCurrentPhaseOverlayVisible(bool bShouldVisible);

    void BindDeptWidgetFunction(UDepthComponent* DepthComp);

    /** HUD 위젯을 숨긴다. */
    void HideHudWidget();

    /** 위젯을 보이게 한다. */
    void ShowHudWidget();

    void SetActiveRadarWidget(bool bShouldActivate);
    
protected:
    
    /** 관전 상태가 변경되었을 때 호출되는 함수 */
    UFUNCTION()
    void OnSpectatingStateChanged(bool bIsSpectating);

    /** 새로운 Pawn에 HUD 위젯을 설정하고 활성화 한다. */
    void SetupHudWidgetToNewPawn(APawn* NewPawn, APlayerController* PlayerController);

    /** 관전 HUD 위젯을 보이게 한다. */
    void ShowSpectatorHUDWidget();

    /** 관전 HUD 위젯을 숨긴다. */
    void HideSpectatorHUDWidget();
    
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

    /** 관전 HUD 위젯 클래스 */
    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class USpectatorHUDWidget> SpectatorHUDWidgetClass;

    /** 관전 HUD 위젯 인스턴스 */
    UPROPERTY()
    TObjectPtr<class USpectatorHUDWidget> SpectatorHUDWidget;
    
    /** 레이더 HUD 위젯 클래스 */
    UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class URadar2DWidget> Radar2DWidgetClass;

    /** 레이더 HUD 위젯 인스턴스 */
    UPROPERTY()
    TObjectPtr<class URadar2DWidget> Radar2DWidget;

#pragma endregion

#pragma region Getter Setter

public:

    bool IsTestHUDVisible() const;

    UMissionsOnHUDWidget* GetMissionsOnHudWidget() const;
    USoundSubsystem* GetSoundSubsystem();
    UPlayerStatusWidget* GetPlayerStatusWidget() ;

#pragma endregion
};