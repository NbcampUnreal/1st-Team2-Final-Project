#include "Character/Laser/AnimNotify/AnimNotify_StopAllMontage.h"
#include "Character/UnderwaterCharacter.h"
#include "Interactable/Item/Component/EquipUseComponent.h"

void UAnimNotify_StopAllMontage::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	if (AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(Owner))
	{
		const float MontageStopSeconds = 0.f;
		Diver->M_StopAllMontagesOnBothMesh(MontageStopSeconds);
	}
}
