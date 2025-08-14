#include "Monster/Boss/AnimNotify/AnimNotify_WorldCameraShake.h"
#include "Monster/Boss/Boss.h"
#include "Monster/Boss/Effect/CameraControllerComponent.h"

void UAnimNotify_WorldCameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ABoss* Boss = Cast<ABoss>(MeshComp->GetOwner());
	if (!IsValid(Boss)) return;

	Boss->GetCameraControllerComponent()->ShakeWorldCamera(CameraShakeClass);
}
