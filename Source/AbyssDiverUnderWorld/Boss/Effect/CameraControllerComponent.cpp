#include "Boss/Effect/CameraControllerComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Kismet/GameplayStatics.h"

UCameraControllerComponent::UCameraControllerComponent()
{
	static ConstructorHelpers::FClassFinder<UCameraShakeBase> BP_RoarCameraShakeBase(TEXT("/Game/_AbyssDiver/Blueprints/Boss/Effect/BP_RoarCameraShakeBase"));
	if (BP_RoarCameraShakeBase.Succeeded())
	{
		ShortCameraShakeClass = BP_RoarCameraShakeBase.Class;
	}

	static ConstructorHelpers::FClassFinder<UCameraShakeBase> BP_DamagedCameraShakeBase(TEXT("/Game/_AbyssDiver/Blueprints/Boss/Effect/BP_DamagedCameraShakeBase"));
	if (BP_DamagedCameraShakeBase.Succeeded())
	{
		DamagedCameraShakeClass = BP_DamagedCameraShakeBase.Class;
	}

	CustomInnerRadius = 0.0f;
	CustomOuterRadius = 7000.0f;
}

void UCameraControllerComponent::ShakeWorldCamera(TSubclassOf<UCameraShakeBase> CameraShakeClass)
{
	if (!IsValid(CameraShakeClass)) return;

	UGameplayStatics::PlayWorldCameraShake(GetWorld(), CameraShakeClass, GetOwner()->GetActorLocation(),
		CustomInnerRadius, CustomOuterRadius, 1.0f, true);
}

void UCameraControllerComponent::ShakePlayerCamera(AUnderwaterCharacter* PlayerCharacter, float& Scale)
{
	if (!IsValid(PlayerCharacter)) return;

	APlayerController* Controller = Cast<APlayerController>(PlayerCharacter->GetController());
	if (!IsValid(Controller)) return;

	Controller->ClientStartCameraShake(DamagedCameraShakeClass, Scale);
}
