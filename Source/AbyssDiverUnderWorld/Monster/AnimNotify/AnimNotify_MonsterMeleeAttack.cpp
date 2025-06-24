// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/AnimNotify/AnimNotify_MonsterMeleeAttack.h"
#include "Monster/Monster.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Character/StatComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/SphereComponent.h"

void UAnimNotify_MonsterMeleeAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	AMonster* Monster = Cast<AMonster>(OwnerActor);
	USphereComponent* HitComponent = Monster ? Monster->GetAttackHitComponent() : nullptr;
	if (!HitComponent || !Monster) return;

	const UStatComponent* StatComponent = Monster->FindComponentByClass<UStatComponent>();
	if (!StatComponent) return;

	// Overlap
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Monster);

	OwnerActor->GetWorld()->OverlapMultiByChannel(
		Overlaps,
		HitComponent->GetComponentLocation(),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(HitComponent->GetScaledSphereRadius()),
		Params
	);

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* HitActor = Result.GetActor();
		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(HitActor);
		if (!Player) return;
		if (Player->GetCharacterState() == ECharacterState::Death) return;
		UGameplayStatics::ApplyDamage(HitActor, StatComponent->AttackPower, Monster->GetController(), Monster, nullptr);
	}
	
}
