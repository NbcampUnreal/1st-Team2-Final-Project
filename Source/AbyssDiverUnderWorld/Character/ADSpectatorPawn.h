// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/SpectatorPawn.h"
#include "ADSpectatorPawn.generated.h"

class UInputAction;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AADSpectatorPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region Method

public:
	
	/** 다음 플레이어를 관전한다. */
	void ViewNextPlayer();

	/** 이전 플레이어를 관전한다. */
	void ViewPrevPlayer();

#pragma endregion

#pragma region Variable

	// 현재 IMC를 분리하고 있지 않다. 추후 분리를 고려할 것.

	/** 다음 플레이어 보기 Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ViewNextPlayerAction;

	/** 이전 플레이어 보기 Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ViewPrevPlayerAction;

#pragma endregion
};
