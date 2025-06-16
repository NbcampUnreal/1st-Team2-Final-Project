#include "Character/Laser/AnimNotify/AnimNotify_PlayWeaponIlde.h"
#include "Character/UnderwaterCharacter.h"
#include "Interactable/Item/Component/EquipUseComponent.h"

void UAnimNotify_PlayWeaponIlde::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	if (AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(Owner))
	{
		if (UEquipUseComponent* EquipComp = Diver->FindComponentByClass<UEquipUseComponent>())
		{
			const float MontageStopSeconds = 0.f;
			Diver->M_StopAllMontagesOnBothMesh(MontageStopSeconds);
			EquipComp->PlayDrawAnimation(Diver);
		}
	}
}
