#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "Tutorial/TutorialEnums.h"
#include "ADPlayerController.generated.h"


enum class ESFX : uint8;
enum class EMapName : uint8;
class ULoadingScreenWidget;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AADPlayerController();
	
protected:

	virtual void BeginPlay() override;
	virtual void SetPawn(APawn* InPawn) override;
	virtual void PostNetInit() override;
	virtual void PostSeamlessTravel() override;

#pragma region Method

public:

	/** 입력 무시할 때, Jump 입력 상태 초기화 */
	virtual void SetIgnoreMoveInput(bool bNewMoveInput) override;
	
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

	/** Camera Blink를 시작한다. FadeAlpha의 X값은 시작 알파 값이고 Y값은 종료 알파 값이다.
	 * FadeStartTime 동안 종료 알파 값으로 Fade Out을 한다.
	 * FadeEndDelay 동안 대기한다.
	 * FadeOut 완료되면 FadeEndTime 동안 FadeColor로 Fade In을 한다.
	*/
	UFUNCTION(Reliable, Client)
	void C_StartCameraBlink(FColor FadeColor, FVector2D FadeAlpha, float FadeStartTime, float FadeEndDelay, float FadeEndTime);
	void C_StartCameraBlink_Implementation(FColor FadeColor, FVector2D FadeAlpha, float FadeStartTime, float FadeEndDelay, float FadeEndTime);

	/** Camera Blink가 진행 중인지 여부를 반환한다. */
	UFUNCTION(BlueprintCallable)
	bool IsCameraBlinking() const;

	/** Camera Blink를 즉시 중단한다. Fade In이 진행 중일 경우에도 중단된다. */
	UFUNCTION(Reliable, Client)
	void C_StopCameraBlink();
	void C_StopCameraBlink_Implementation();

	UFUNCTION(Client, Unreliable)
	void C_PlaySound(ESFX SoundType, float VolumeMultiplier = 1.0f);

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

	/** 마우스 X축 입력에 감도 적용 */
	virtual void AddYawInput(float Val) override;

	/** 마우스 Y축 입력에 감도 적용 */
	virtual void AddPitchInput(float Val) override;
	
	/** 마우스 감도 설정 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Control")
	void SetLookSensitivity(float NewXSensitivity, float NewYSensitivity);

	UFUNCTION()
	void ShowPauseMenu();

	UFUNCTION()
	void HidePauseMenu();

	UFUNCTION()
	void TogglePauseMenu();

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

	/** 이름 위젯의 표시 상태를 토글 */
	UFUNCTION(Exec)
	void ToggleNameWidget();

	/** 이름 위젯을 표시하도록 설정 */
	UFUNCTION(Exec)
	void ShowNameWidgets();

	/** 이름 위젯을 숨기도록 설정 */
	UFUNCTION(Exec)
	void HideNameWidgets();

	/** 조준선 위젯의 표시 상태를 토글 */
	UFUNCTION(Exec)
	void ToggleCrosshairWidget();

	/** 조준선 위젯을 표시하도록 설정 */
	UFUNCTION(Exec)
	void ShowCrosshairWidget();

	/** 조준선 위젯을 숨기도록 설정 */
	UFUNCTION(Exec)
	void HideCrosshairWidget();

	/** ItemId에 해당하는 아이템을 인벤토리에 추가 (테스트용) */
	UFUNCTION(Exec)
	void GetItemById(uint8 ItemId);

	/** Server에서 ItemId에 해당하는 아이템을 인벤토리에 추가 */
	UFUNCTION(Server, Reliable)
	void S_GetItemById(uint8 ItemId);
	void S_GetItemById_Implementation(uint8 ItemId);
	
	UFUNCTION(Server, Reliable)
	void S_GainShield(int Amount);
	
	void OnCameraBlankEnd();
private:
	
	/** 이름 위젯 가시 상태 설정 */
	void SetAllNameWidgetsEnabled(bool bNewEnabled);

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InventoryAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_Pause;

	UPROPERTY()
	uint8 bIsPauseMenuOpened : 1 = 0;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

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

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UPauseWidget> PauseWidgetClass;

	UPROPERTY()
	TObjectPtr<UPauseWidget> PauseWidgetInstance;


	/** Camera Blank Timer Handle */
	FTimerHandle CameraBlankTimerHandle;

	/** Name Widget 가시성 표시 여부. 비활성화되면 모든 Name Widget이 숨김 처리된다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	uint8 bIsNameWidgetEnabled : 1;

	/** 조준선 위젯의 가시성 표시 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	uint8 bIsCrosshairWidgetVisible : 1 = true;

	/** 마우스 X축 감도 */
	UPROPERTY(BlueprintReadOnly, Category = "Control", meta = (AllowPrivateAccess = "true"))
	float MouseXSensitivity = 1.0f;

	/** 마우스 Y축 감도 */
	UPROPERTY(BlueprintReadOnly, Category = "Control", meta = (AllowPrivateAccess = "true"))
	float MouseYSensitivity = 1.0f;

#pragma endregion 

#pragma region Getters / Setters

public:

	UPlayerHUDComponent* GetPlayerHUDComponent() const { return PlayerHUDComponent; }

	/** Name Widget이 활성화 여부를 반환 */
	FORCEINLINE bool IsNameWidgetEnabled() const { return bIsNameWidgetEnabled; }

	/** 마우스 X축, Y축 감도를 반환 */
	FVector2D GetMouseSensitivity() const { return FVector2D(MouseXSensitivity, MouseYSensitivity); }
	
	/** 마우스 X축 감도를 반환 */
	float GetMouseXSensitivity() const { return MouseXSensitivity; }

	/** 마우스 Y축 감도를 반환 */
	float GetMouseYSensitivity() const { return MouseYSensitivity; }
	
#pragma endregion

};
