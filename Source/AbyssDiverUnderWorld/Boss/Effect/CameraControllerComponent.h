#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraControllerComponent.generated.h"

class AUnderwaterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UCameraControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCameraControllerComponent();

public:
	/** 애니메이션 노티파이로 활용할 Shaking 함수
	 *
	 * Notify로 호출할 것이기 때문에 네트워크 기능 불필요 */
	void ShakeWorldCamera(TSubclassOf<UCameraShakeBase> CameraShakeClass);

	/** 보스에게 공격당한 플레이어의 카메라만 Shaking 하는 함수 */
	void ShakePlayerCamera(AUnderwaterCharacter* PlayerCharacter);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Camera")
	TSubclassOf<UCameraShakeBase> ShortCameraShakeClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Camera")
	TSubclassOf<UCameraShakeBase> DamagedCameraShakeClass;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Camera|Setting")
	float CustomInnerRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Camera|Setting")
	float CustomOuterRadius;
	
};