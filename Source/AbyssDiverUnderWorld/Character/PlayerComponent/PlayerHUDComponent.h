// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/PlayerStatusWidget.h"
#include "PlayerHUDComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UPlayerHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerHUDComponent();

protected:
	UFUNCTION()
	// Called when the game starts
	virtual void BeginPlay() override;

#pragma region Method

public:

	UFUNCTION(Client, Reliable)
	void C_ShowResultScreen();
	void C_ShowResultScreen_Implementation();

	/** 현재 HUD 보이는 것 설정 */
	void SetVisibility(bool NewVisible) const;

	void SetResultScreenVisible(const bool bShouldVisible) const;
	void UpdateResultScreen(int32 PlayerIndexBased_1, const struct FResultScreenParams& Params);

private:

	/** OnPossessedPawnChanged 바인딩 함수. Respawn 등이 되었다면 새로 바인딩을 시작한다. */
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void UpdateOxygenHUD(float CurrentOxygenLevel, float MaxOxygenLevel);
#pragma endregion

#pragma region Variable

private:

	/** HUD 생성 클래스 지정 */
	UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UPlayerHUDWidget> HudWidgetClass;

	/** 현재 생성된 HUD 위젯 */
	UPROPERTY()
	TObjectPtr<class UPlayerHUDWidget> HudWidget;

	/** 게임 결과 위젯 */
	UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UResultScreen> ResultScreenWidgetClass;

	UPROPERTY()
	TObjectPtr<class UResultScreen> ResultScreenWidget;
	
	// 산소 위젯 클래스 (에디터에서 설정)
	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UPlayerStatusWidget> PlayerStatusWidgetClass;

	// 실제 생성된 산소 위젯 인스턴스
	UPROPERTY()
	TObjectPtr<UPlayerStatusWidget> PlayerStatusWidget;
#pragma endregion
};
