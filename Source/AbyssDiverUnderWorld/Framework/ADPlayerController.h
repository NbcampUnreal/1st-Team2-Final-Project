#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"

#include "ADPlayerController.generated.h"

enum class ESFX : uint8;
enum class EMapName : uint8;
class ULoadingScreenWidget;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADPlayerController : public APlayerController
{
	GENERATED_BODY()

	AADPlayerController();
	
protected:

	virtual void BeginPlay() override;
	virtual void SetPawn(APawn* InPawn) override;
	virtual void PostNetInit() override;
	virtual void PostSeamlessTravel() override;

#pragma region Method

public:

	UFUNCTION(Client, Reliable)
	void C_OnPreClientTravel();
	void C_OnPreClientTravel_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPreClientTravel();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPostNetInit();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPostSeamlessTravel();

	UFUNCTION(Client, Reliable)
	void C_PlayGameOverSound();
	void C_PlayGameOverSound_Implementation();

	UFUNCTION()
	void ShowFadeOut(float Duration = 2.0f);

	UFUNCTION()
	void ShowFadeIn();

	/** Pawn이 변경되었을 때 호출된다. Client에서 관전 시작 시에 시점 조정을 위해 사용 */
	virtual void OnRep_Pawn() override;
	
	/** Player가 관전을 하도록 한다. Server에서만 작동하며 Client도 관전을 진행하게 한다. */
	void StartSpectate();

	/** View Target 변경 Delegate를 호출하기 위해 오버라이딩을 한다.
	 * Server에서 호출될 경우 현재 자신이 로컬이 아닐 경우 Client에게 RPC를 호출해서 일치시킨다.
	 * 즉, Server에서 모든 Player Controller에서 Delegate를 호출하게 된다.
	 * Client에서는 Local Player Controller의 Delegate를 호출하게 된다.
	 * UI를 갱신하기 위해서는 Local Player Controller에서만 호출되도록 한다.
	 */
	virtual void SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;

	/** Camera Blank를 시작한다. FadeAlpha의 X값은 시작 알파 값이고 Y값은 종료 알파 값이다.
	 * FadeTime 동안 종료 알파 값으로 Fade Out을 한다.
	 * FadeOut 완료되면 FadeTime 동안 FadeColor로 Fade In을 한다.
	*/
	UFUNCTION(Reliable, Client)
	void C_StartCameraBlink(FColor FadeColor, FVector2D FadeAlpha, float FadeStartTime, float FadeEndDelay, float FadeEndTime);
	void C_StartCameraBlink_Implementation(FColor FadeColor, FVector2D FadeAlpha, float FadeStartTime, float FadeEndDelay, float FadeEndTime);

	UFUNCTION(BlueprintCallable)
	bool IsCameraBlanking() const;

	UFUNCTION(Client, Unreliable)
	void C_PlaySound(ESFX SoundType, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

	UFUNCTION(Exec)
	void ShowPlayerHUD();

	UFUNCTION(Exec)
	void HidePlayerHUD();

	UFUNCTION(Exec)
	void SetInvincible(bool bIsInvincible);

	UFUNCTION(Server, Reliable)
	void S_SetInvincible(bool bIsInvincible);

	UFUNCTION(Exec)
	void KillPlayer();

	UFUNCTION(Server, Reliable)
	void S_KillPlayer();
	void S_KillPlayer_Implementation();

	void SetActiveRadarWidget(bool bShouldActivate);

	UFUNCTION(Server, Reliable)
	void Server_RequestAdvanceTutorialPhase();

	UFUNCTION(Server, Reliable)
	void S_UnlockMonster(FName MonsterId);
	void S_UnlockMonster_Implementation(FName MonsterId);


protected:

	/** 관전 상태가 시작될 때 호출되는 함수 */
	virtual void BeginSpectatingState() override;

	/** 관전 종료 시에 호출되는 함수 */
	virtual void EndSpectatingState() override;

	UFUNCTION(Server, Reliable)
	void S_RequestSelectLevel(const EMapName InLevelName);
	void S_RequestSelectLevel_Implementation(const EMapName InLevelName);

	UFUNCTION(Server, Reliable)
	void S_RequestStartGame();
	void S_RequestStartGame_Implementation();

	virtual void SetupInputComponent() override;
	void ShowInventory(const FInputActionValue& InputActionValue);
	void HideInventory(const FInputActionValue& InputActionValue);

	UFUNCTION(Exec)
	void ToggleTestHUD();

	UFUNCTION(Exec)
	void GainShield(int Amount);

	UFUNCTION(Server, Reliable)
	void S_GainShield(int Amount);
	
	void OnCameraBlankEnd();

	UFUNCTION()
	void OnInventoryTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnSprintTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnRadarTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnLightToggleTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnLootingTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnDroneTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnItemsTriggered(const FInputActionValue& Value);
	UFUNCTION()
	void OnReviveTriggered(const FInputActionValue& Value);

	void CheckTutorialObjective(const FInputActionValue& Value, UInputAction* SourceAction);
	
	void ObserveToggle(const FInputActionValue& Value);
	void ObserveCapture(const FInputActionValue& Value);

	void ApplyObserveVisuals(bool bEnable);
	void ToggleObserveIMC(bool bEnable);

	void InitObserveCameraDefaults();
#pragma endregion
	
#pragma region Variable


public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpectateChanged, bool, bIsSpectating);
	/** 관전 상태가 변경될 때 호출되는 Delegate */
	UPROPERTY(BlueprintAssignable, Category = "Spectate")
	FOnSpectateChanged OnSpectateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetViewChanged, AActor*, NewViewTarget);
	/** View Target가 변경될 때 호출되는 Delegate. SetViewTarget를 주석을 확인할 것 */
	UPROPERTY(BlueprintAssignable, Category = "ViewTarget")
	FOnTargetViewChanged OnTargetViewChanged;
	
private:
	uint8 bObserveMode : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> IMC_Observe;


	// 우선순위
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	int32 Priority_Default = 10;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	int32 Priority_Observe = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InventoryAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UPlayerHUDComponent> PlayerHUDComponent;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UInteractionDescriptionWidget> InteractionWidgetClass;

	UPROPERTY()
	TObjectPtr<UInteractionDescriptionWidget> InteractionWidget;


	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UHoldInteractionWidget> InteractionHoldWidgetClass;

	UPROPERTY()
	TObjectPtr<UHoldInteractionWidget> InteractionHoldWidget;

	/** Camera Blank Timer Handle */
	FTimerHandle CameraBlankTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> RadarAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LightToggleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LootingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DroneAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ItemsAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ReviveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ObserveToggleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ObserveCaptureAction;

	UPROPERTY(EditDefaultsOnly, Category = "Observe|Camera")
	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Observe|Camera")
	float ObserveFOV = 75.f;
#pragma endregion 

#pragma region Getters / Setters

public:

	UPlayerHUDComponent* GetPlayerHUDComponent() const { return PlayerHUDComponent; }

#pragma endregion

};
