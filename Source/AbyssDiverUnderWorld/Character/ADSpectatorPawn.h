// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ADInGameMode.h"
#include "GameFramework/SpectatorPawn.h"
#include "ADSpectatorPawn.generated.h"

class UInputAction;

// Spectator Pawn은 Replicate 되지 않으므로 Authority를 확인할려면 NetMode를 확인해야 한다.
DECLARE_LOG_CATEGORY_EXTERN(LogAbyssDiverSpectate, Log, All);

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
	virtual void Destroyed() override;
	
public:
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;
	
#pragma region Method

public:
	
	/** 다음 플레이어를 관전한다. */
	void ViewNextPlayer();

	/** 이전 플레이어를 관전한다. */
	void ViewPrevPlayer();

protected:
	
	/** 타겟 뷰가 변경되었을 때 호출되는 함수 */
	UFUNCTION()
	void OnTargetViewChanged(AActor* NewViewTarget);

	/** 캐릭터 상태가 변경되었을 때 호출되는 함수 */
	UFUNCTION()
	void OnCharacterStateChanged(ECharacterState OldCharacterState, ECharacterState NewCharacterState);
	
#pragma endregion

#pragma region Variable

	// 현재 IMC를 분리하고 있지 않다. 추후 분리를 고려할 것.

	/** 다음 플레이어 보기 Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ViewNextPlayerAction;

	/** 이전 플레이어 보기 Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ViewPrevPlayerAction;

	TWeakObjectPtr<class AUnderwaterCharacter> PrevTargetCharacter;
	
#pragma endregion
};
