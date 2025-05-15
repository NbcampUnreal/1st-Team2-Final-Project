// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

	/** 현재 HUD 보이는 것 설정 */
	void SetVisibility(bool NewVisible) const;

private:

	/** OnPossessedPawnChanged 바인딩 함수. Respawn 등이 되었다면 새로 바인딩을 시작한다. */
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
#pragma endregion

#pragma region Variable

private:

	/** HUD 생성 클래스 지정 */
	UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UPlayerHUDWidget> HudWidgetClass;

	/** 현재 생성된 HUD 위젯 */
	UPROPERTY()
	TObjectPtr<class UPlayerHUDWidget> HudWidget;

	
#pragma endregion
};
