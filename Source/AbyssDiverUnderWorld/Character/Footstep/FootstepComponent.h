// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotify_Footstep.h"
#include "FootstepTypes.h"
#include "Components/ActorComponent.h"
#include "FootstepComponent.generated.h"

enum class ESFX : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFootstepComponent();

protected:
	virtual void BeginPlay() override;

#pragma region Method

public:

	/** 시작 위치에서 발소리를 재생한다. */
	void PlayFootstepSound(const FVector& StartLocation);

	/** FootSide에 따라 소리를 재생한다. Socket 이름이 정의되어 있지 않을 경우 Actor Location에서 소리를 재생한다. */
	void PlayFootstepSound(EFootSide FootSide);

	/** 지상 착지 시에 소리를 재생한다. */
	void PlayLandSound();

protected:

	/** 시작 위치에서 발소리를 재생하는 내부 함수. 실제 Sound를 재생한다. */
	void PlayFootstepSoundInternal(const FVector& StartLocation);

	/** 시작 위치로부터 표면 타입을 반환한다. */
	EPhysicalSurface GetSurfaceType(const FVector& Start) const;

	/** 표면 타입에 맞는 소리를 찾아 반환한다. */
	ESFX FindSound(EPhysicalSurface SurfaceType, EFootstepType FootstepType) const;

	/** 착지 시에 호출되는 함수. Character의 OnLanded 이벤트를 사용한다. */
	UFUNCTION()
	virtual void OnLanded(const FHitResult& Hit);

	class USoundSubsystem* GetSoundSubsystem();
	
#pragma endregion

#pragma region Variable

private:
	
	/** 소유 캐릭터 */
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	/** Character의 OnLanded 이벤트에 착지 사운드 재생 여부. 착지를 AnimNotify로 처리할 경우 false로 설정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Footstep", meta = (AllowPrivateAccess = "true"))
	bool bUseCharacterOnLanded;

	/** 최소 발소리 재생 간격. 발소리가 겹쳐서 재생되지 않도록 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Footstep", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinimumFootstepInterval;
	
	/** 마지막 발소리 재생 시간. 발소리 재생 간격을 조절한다. */
	float LastFootstepTime;
	
	/** Trace Location에서 Height만큼 위로 올라간 위치에서 Trace를 시작한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Footstep", meta = (AllowPrivateAccess = "true"))
	float TraceHeight;
	
	/** Trace Location에서 DownVector 방향으로 TraceDistance만큼 떨어진 위치까지 Trace를 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Footstep", meta = (AllowPrivateAccess = "true"))
	float TraceDistance;

	/** 걷기 발소리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Footstep", meta = (AllowPrivateAccess = "true"))
	ESFX WalkFootstepSound;

	/** 착지 발소리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Footstep", meta = (AllowPrivateAccess = "true"))
	ESFX LandFootstepSound;
	
	/** 왼발 Socket 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character|Footstep", meta = (AllowPrivateAccess = "true"))
	FName LeftFootSocketName;

	/** 오른발 Socket 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character|Footstep", meta = (AllowPrivateAccess = "true"))
	FName RightFootSocketName;

	TWeakObjectPtr<class USoundSubsystem> SoundSubsystem;
	
#pragma endregion
};
