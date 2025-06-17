// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class URichTextBlock;
enum class ECharacterState : uint8;

/**
 * Player HUD를 표시하는 Widget입니다. 태스트 UI를 출력하고 있습니다.
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
#pragma region Method

public:
	void BindWidget(APawn* PlayerPawn);

	/** 체력 택스트를 갱신 */
	UFUNCTION(BlueprintCallable)
	void UpdateHealthText(int32 Health, int32 MaxHealth);

	/** 실드 택스트를 갱신 */
	UFUNCTION(BlueprintCallable)
	void UpdateShieldText(float OldShieldValue, float NewShieldValue);

	/** 산소 택스트를 갱신 */
	UFUNCTION(BlueprintCallable)
	void UpdateOxygenText(float Oxygen, float MaxOxygen);

	/** 스태미나 택스트를 갱신 */
	UFUNCTION(BlueprintCallable)
	void UpdateStaminaText(float Stamina, float MaxStamina);

	/** Character State를 갱신 */
	UFUNCTION(BlueprintCallable)
	void UpdateCharacterStateText(ECharacterState CharacterState);

	/** Groggy 시간을 갱신 */
	UFUNCTION(BlueprintCallable)
	void UpdateGroggyText(float GroggyTime, float MaxGroggyTime);

	UFUNCTION()
	void UpdateCombatText(bool bIsInCombat);

protected:

	/** Character State 변화 처리 */
	UFUNCTION()
	void OnCharacterStateChanged(ECharacterState OldCharacterState, ECharacterState NewCharacterState);

	/** Combat 시작 시 처리 */
	UFUNCTION()
	void OnCombatStart();

	/** Combat 종료 시 처리 */
	UFUNCTION()
	void OnCombatEnd();
	
#pragma endregion

#pragma region Variable

private:

	/** 체력 택스트, 임시용이고 디버깅용으로 수정 예정 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> HealthTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> ShieldTextBlock;

	/** 산소 택스트, 임시용이고 디버깅용으로 수정 예정 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> OxygenTextBlock;

	/** 스태미나 택스트, 임시용이고 디버깅용으로 수정 예정 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> StaminaTextBlock;

	/** 캐릭터 스테이트 택스트, 디버깅용으로 수정 예정 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> CharacterStateTextBlock;

	/** 그로기 시간 텍스트, 디버깅용으로 수정 예정 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> GroggyTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> CombatTextBlock;
	
#pragma endregion
	
};
