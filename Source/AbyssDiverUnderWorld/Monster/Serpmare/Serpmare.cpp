﻿#include "Monster/Serpmare/Serpmare.h"

#include "Monster/Components/AquaticMovementComponent.h"
#include "Monster/Components/TickControlComponent.h"

#include "Character/UnderwaterCharacter.h"

ASerpmare::ASerpmare()
{
	LowerBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LowerBodyMesh"));
	LowerBodyMesh->SetupAttachment(GetMesh());

	WeakPointMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeakPointMesh"));
	WeakPointMesh->SetupAttachment(GetMesh());
}

void ASerpmare::BeginPlay()
{
	Super::BeginPlay();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &ASerpmare::OnMeshOverlapBegin);

	TickControlComponent->RegisterComponent(LowerBodyMesh);
	TickControlComponent->RegisterComponent(WeakPointMesh);

	if (AquaticMovementComponent)
	{
		TickControlComponent->UnregisterComponent(AquaticMovementComponent);
	}
}

void ASerpmare::Attack()
{
	// Serpmare는 Detection 상태에서도 AM가 발동중이기 때문에 Monster 클래스처럼 Early Return하면 공격을 못 함.

	const uint8 AttackType = FMath::RandRange(0, AttackAnimations.Num() - 1);
	if (IsValid(AttackAnimations[AttackType]))
	{
		M_PlayMontage(AttackAnimations[AttackType]);
	}

	bIsAttacking = true;

	GetWorldTimerManager().SetTimer(AttackIntervalTimer, this, &ASerpmare::InitAttackInterval, AttackInterval, false);
	bCanAttack = false;
}

void ASerpmare::AddDetection(AActor* Actor)
{
	if (!IsValid(Actor) || !IsValid(this)) return;
	if (!HasAuthority()) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
	if (!Player) return;

	if (!DetectedPlayers.Contains(Actor))
	{
		DetectedPlayers.Add(Actor);

		UE_LOG(LogTemp, Log, TEXT("[%s] AddDetection : %s | ArraySize: %d"),
			*GetName(),
			*Actor->GetName(),
			DetectedPlayers.Num()
		);

		Player->OnTargeted(this);
	}
	else
	{
		return;
	}

	// 기존에 MonsterState를 Chase로 바꿔주었으나 Serpmare는 BT와 AM 상에서 모든 State를 전환함. 그래서 필요 없음
}


void ASerpmare::RemoveDetection(AActor* Actor)
{
	if (!IsValid(this) || !IsValid(Actor)) return;
	if (!HasAuthority()) return;

	// DetectedPlayers에 Actor가 없으면 return;
	if (!DetectedPlayers.Remove(Actor)) return;

	UE_LOG(LogTemp, Log, TEXT("[%s] RemoveDetection : %s | ArraySize: %d"),
		*GetName(),
		*Actor->GetName(),
		DetectedPlayers.Num())

		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
	if (!Player)
	{
		LOG(TEXT("Player가 유효하지 않음"));
		return;
	}

	Player->OnUntargeted(this);

	// 만약 현재 설정된 TargetPlayer가 빠졌다면 다른 타겟 지정해야함
	const bool bWasTarget = (TargetPlayer.Get() == Actor);
	if (bWasTarget)
	{
		TargetPlayer.Reset();
	}

	// 기존에 MonsterState를 Patrol로 바꿔주었으나 Serpmare는 BT와 AM 상에서 모든 State를 전환함. 그래서 필요 없음
}

void ASerpmare::InitAttackInterval()
{
	bCanAttack = true;
}
