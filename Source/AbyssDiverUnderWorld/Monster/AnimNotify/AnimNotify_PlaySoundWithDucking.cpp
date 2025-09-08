// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/AnimNotify/AnimNotify_PlaySoundWithDucking.h"
#include "Kismet/GameplayStatics.h"
#include "Monster/Components/MonsterSoundComponent.h"

void UAnimNotify_PlaySoundWithDucking::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!SoundToPlay || !MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// Sound Local Play
	UGameplayStatics::PlaySoundAtLocation(Owner, SoundToPlay, Owner->GetActorLocation());

	// Request the server to reduce the main volume
	if (UMonsterSoundComponent* SoundComp = Owner->FindComponentByClass<UMonsterSoundComponent>())
	{
		SoundComp->S_RequestMainSoundDuck(DuckVolume, DuckDuration, RecoverDuration);
	}
}
