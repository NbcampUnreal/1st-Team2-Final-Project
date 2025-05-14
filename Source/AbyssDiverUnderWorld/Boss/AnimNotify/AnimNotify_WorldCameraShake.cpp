#include "Boss/AnimNotify/AnimNotify_WorldCameraShake.h"
#include "Boss/Boss.h"
#include "Boss/Effect/CameraControllerComponent.h"

void UAnimNotify_WorldCameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ABoss* Boss = Cast<ABoss>(MeshComp->GetOwner());
	if (!IsValid(Boss)) return;

	Boss->GetCameraControllerComponent()->ShakeWorldCamera(CameraShakeClass);
}
