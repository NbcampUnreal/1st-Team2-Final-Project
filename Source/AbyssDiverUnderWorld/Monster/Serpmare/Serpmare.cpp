#include "Monster/Serpmare/Serpmare.h"

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

float ASerpmare::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.0f;
	}

	const float Damage = AUnitBase::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (IsValid(StatComponent))
	{
		FVector BloodLoc = GetMesh()->GetComponentLocation() + FVector(0, 0, 20.f);
		FRotator BloodRot = GetActorRotation();
		M_SpawnBloodEffect(BloodLoc, BloodRot);

		if (StatComponent->GetCurrentHealth() <= 0)
		{
			OnDeath();
			// Delegate Broadcasts for Achievements
			OnMonsterDead.Broadcast(DamageCauser, this);
		}
		else
		{
			if (IsValid(HitReactAnimations))
			{
				M_PlayMontage(HitReactAnimations);
			}
			else if (MonsterSoundComponent)
			{
				MonsterSoundComponent->S_PlayHitReactSound();
			}

			// Serpmare는 피해를 입어도 어그로 끌리지 않음
		}
	}

	return Damage;
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

void ASerpmare::NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor)
{
	// Serpmare는 빛에 반응 하지 않음
}

void ASerpmare::AddDetection(AActor* Actor)
{
	if (!IsValid(Actor) || !IsValid(this)) return;
	if (!HasAuthority()) return;

	if (GetMonsterState() == EMonsterState::Death)
	{
		return;
	}

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

	// 만약 TargetPlayer가 없으면 TargetPlayer를 해당 Actor(Player)로 설정.
	if (!TargetPlayer.IsValid())
	{
		TargetPlayer = Player;

		if (BlackboardComponent)
		{
			SetTarget(TargetPlayer.Get());
		}
	}
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

		if (BlackboardComponent)
		{
			SetTarget(nullptr);
		}

		if (DetectedPlayers.Num() == 0)
		{
			return;
		}

		// TSet 순회하여 요소(Player)가 남아있으면 해당 플레이어를 TargetPlayer로 지정
		for (const TWeakObjectPtr<AActor>& Elem : DetectedPlayers)
		{
			if (AActor* NewTarget = Elem.Get())
			{
				AUnderwaterCharacter* DetectedPlayer = Cast<AUnderwaterCharacter>(NewTarget);

				if (DetectedPlayer)
				{
					TargetPlayer = DetectedPlayer;
				}

				if (BlackboardComponent)
				{
					//BlackboardComponent->SetValueAsObject(BlackboardKeys::TargetPlayerKey, TargetPlayer.Get());
					SetTarget(TargetPlayer.Get());
				}
				return;
			}
		}
	}
}

void ASerpmare::ReceiveKnockback(const FVector& Force)
{
	// 넉백 당하지 않음.
}

void ASerpmare::OnAttackCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	LOGV(Error, TEXT("Overlap Begin !(%s)"), *OtherActor->GetName());
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!Player) return;

	AddDetection(Player);
}

void ASerpmare::OnAttackCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	LOGV(Error, TEXT("Overlap End !(%s)"), *OtherActor->GetName());
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!Player) return;

	RemoveDetection(Player);
}

void ASerpmare::InitAttackInterval()
{
	bCanAttack = true;
}
