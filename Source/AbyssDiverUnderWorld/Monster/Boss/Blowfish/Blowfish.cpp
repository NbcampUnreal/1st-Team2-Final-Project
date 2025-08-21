#include "Monster/Boss/Blowfish/Blowfish.h"
#include "AbyssDiverUnderWorld.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/StatComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"

ABlowfish::ABlowfish()
{
	BiteCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BiteCollision"));
	BiteCollision->SetupAttachment(GetMesh(), TEXT("BiteSocket"));
	BiteCollision->SetCapsuleHalfHeight(50.0f);
	BiteCollision->SetCapsuleRadius(50.0f);
	BiteCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BiteCollision->ComponentTags.Add(TEXT("BiteCollision"));

	TailCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("TailCollision"));
	TailCollision->SetupAttachment(GetMesh(), TEXT("TailSocket"));
	TailCollision->SetCapsuleHalfHeight(50.0f);
	TailCollision->SetCapsuleRadius(50.0f);
	TailCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TailCollision->ComponentTags.Add(TEXT("TailCollision"));

	ExplosionHealthRatio = 0.3f;
	bIsExplosionTriggered = false;
	ExplosionDelayTime = 2.0f;
	ExplosionTriggeredMovementMultiplier = 3.5f;
}

void ABlowfish::BeginPlay()
{
	Super::BeginPlay();

	BiteCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
	TailCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnMeshOverlapBegin);
}

float ABlowfish::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (StatComponent->CurrentHealth <= StatComponent->MaxHealth * ExplosionHealthRatio
		&& StatComponent->CurrentHealth > 0.0f
		&& !bIsExplosionTriggered)
	{
		M_TriggerExplosion();
	}

	return Damage;
}

void ABlowfish::OnDeath()
{
	GetWorldTimerManager().ClearTimer(ExplosionTimerHandle);
	
	Super::OnDeath();
}

void ABlowfish::M_TriggerExplosion_Implementation()
{
	// 복어 스케일 커지는 타임라인 트리거
	ScaleUpTriggered();

	// 이동 속도를 ExplosionTriggeredMovementMultiplier만큼 곱한 값으로 증가
	// 임시 BrakingDeceleration 설정, 기존 0이면 이동 자체를 하지 않음.
	SetCharacterMovementSetting(111.0f, StatComponent->GetMoveSpeed() * ExplosionTriggeredMovementMultiplier);

	// Explosion을 1회성으로 호출하기 위한 bool 값 활성화
	bIsExplosionTriggered = true;

	// ExplosionDelayTime이 경과 후 폭발 로직 수행
	GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &ABlowfish::Explosion, ExplosionDelayTime, false);
}

void ABlowfish::Explosion()
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	GetWorld(), BloodEffect, GetActorLocation(), FRotator::ZeroRotator, FVector(1), true, true );
	
	// 폭발 디버그 구체 그리기 (파란색, 1초 동안 표시)
	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		ExplosionRadius,
		32,                    // 세그먼트 수 (더 높이면 원형이 매끄러워짐)
		FColor::Red,           // 색상
		false,                 // 지속 여부 (true: 영구)
		1.0f                   // 표시 시간 (초)
	);

	TArray<FOverlapResult> Overlaps;

	// 충돌 설정: Pawn 채널 탐지
	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ExplosionRadius),
		Params
	);

	// 충돌 범위 내에 있는 플레이어들 넉백
	for (const FOverlapResult& Result : Overlaps)
	{
		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Result.GetActor());
		if (IsValid(Player))
		{
			float Power = LaunchPower * 5.0f;
			LaunchPlayer(Player, Power);
		}
	}
	
	// 복어 위치를 기준으로 ExplosionRadius 범위내에 있는 액터에게 ExplosionDamage를 가함
	// ECC_WorldStatic으로 프리셋이 설정된 액터에 가려진 경우 데미지를 받지 않음
	UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius,
		UDamageType::StaticClass(), TArray<AActor*>(), this, GetController(), false, ECollisionChannel::ECC_WorldStatic);
	
	Destroy(true);
}
