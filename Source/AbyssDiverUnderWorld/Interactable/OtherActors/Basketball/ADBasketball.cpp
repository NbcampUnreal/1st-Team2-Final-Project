#include "Interactable/OtherActors/Basketball/ADBasketball.h"

#include "Character/UnderwaterCharacter.h"
#include "AbyssDiverUnderWorld.h"
#include "Framework/ADGameInstance.h"

#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/Localizations/LocalizationSubsystem.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Interactable/Item/Component/ADInteractionComponent.h"

// Sets default values
AADBasketball::AADBasketball()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->SetSphereRadius(12.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(CollisionSphere);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BallMesh->SetIsReplicated(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.6f; // 농구공 탄성
	ProjectileMovement->Friction = 0.2f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->SetIsReplicated(true);
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bRotationFollowsVelocity = false;

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));

	CollisionSphere->OnComponentHit.AddDynamic(this, &AADBasketball::OnHit);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AADBasketball::OnBeginOverlap);
}

void AADBasketball::BeginPlay()
{
	Super::BeginPlay();
	
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystemWeakPtr = GI->GetSubsystem<USoundSubsystem>();
	}

	CollisionSphere->SetSimulatePhysics(true);
	ProjectileMovement->SetActive(false);
}

FString AADBasketball::GetInteractionDescription() const
{
	ULocalizationSubsystem* LocalizationSubsystem = GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
	if (!IsValid(LocalizationSubsystem))
	{
		LOGV(Error, TEXT("Can't Get LocalizationSubsystem"));
		return "";
	}
	// 상황에 따른 설명
	if (IsHeld())
	{
		// 이미 누군가 들고 있음
		return TEXT("농구공 (사용 중)");
	}
	else if (bIsThrown)
	{
		// 던져진 상태
		return TEXT("농구공 (날아가는 중)");
	}
	else
	{
		// 줍기 가능
		return LocalizationSubsystem->GetLocalizedText(
			ST_InteractionDescription::TableKey,
			TEXT("Basketball_Pickup") // 로컬라이제이션 키 추가 필요
		).ToString();
	}
}

void AADBasketball::Interact_Implementation(AActor* InstigatorActor)
{
	AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(InstigatorActor);
	if (!Diver) return;

	if (!HeldBy && !bIsThrown)
	{
		Pickup(Diver);
	}
	else if (HeldBy == Diver)
	{
		Throw();
	}
}

void AADBasketball::Pickup(AUnderwaterCharacter* Diver)
{
	if (!Diver || !Diver->GetMesh()) return;

	if (!HasAuthority()) return;

	HeldBy = Diver;
	CachedHeldBy = Diver;
	bIsThrown = false;

	// MovementReplication 꺼서 충돌 방지
	SetReplicateMovement(false);

	CollisionSphere->SetSimulatePhysics(false);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->SetActive(false);
	ProjectileMovement->Velocity = FVector::ZeroVector;

	USkeletalMeshComponent* Mesh1P = Diver->GetMesh();
	AttachToComponent(
		Mesh1P,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		BasketballSocketName
	);
	PlayBasketballAnimation(Diver);

	if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
	{
		SoundSubsystem->PlayAt(ESFX::BasketballPickup, GetActorLocation());
	}
	if (UADInteractionComponent* InteractionComp = Diver->GetInteractionComponent())
	{
		InteractionComp->SetHeldItem(this);
	}

	LOG(TEXT("[%s] Basketball picked up - Role: %d, RemoteRole: %d"),
		HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		(int32)GetLocalRole(),
		(int32)GetRemoteRole());
}

void AADBasketball::Throw()
{
	if (!HeldBy || !HasAuthority())
		return;

	StopBasketballAnimation(HeldBy);
	if (HeldBy)
	{
		if (UADInteractionComponent* InteractionComp = HeldBy->FindComponentByClass<UADInteractionComponent>())
		{
			InteractionComp->SetHeldItem(nullptr);
		}
	}

	FVector ThrowVelocity = CalculateThrowVelocity();
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 던진 다음에는 다시 켜기
	SetReplicateMovement(true);

/*	ProjectileMovement->SetActive(false); 
	ProjectileMovement->StopMovementImmediately(); */ 

	if (AActor* PrevOwner = CachedHeldBy.Get())
	{
		CollisionSphere->IgnoreActorWhenMoving(PrevOwner, true);

		GetWorld()->GetTimerManager().SetTimer(
			OwnerIgnoreTimerHandle,
			this,
			&AADBasketball::ClearOwnerIgnore,
			OwnerIgnoreTime,
			false
		);
	}

	CollisionSphere->SetSimulatePhysics(true);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	/*ProjectileMovement->SetActive(true);
	ProjectileMovement->Velocity = ThrowVelocity;*/
	CollisionSphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	CollisionSphere->SetPhysicsLinearVelocity(ThrowVelocity);

	bIsThrown = true;
	HeldBy = nullptr;

	if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
	{
		SoundSubsystem->PlayAt(ESFX::BasketballThrow, GetActorLocation());
	}

	GetWorld()->GetTimerManager().SetTimer(
		PickupEnableTimerHandle,
		this,
		&AADBasketball::EnablePickupAfterThrow,
		PickupCooldown,
		false
	);

	LOG(TEXT("Basketball thrown with velocity: %s"), *ThrowVelocity.ToString());
}

void AADBasketball::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	LOG(TEXT("[HIT] Self=%s | HitComp=%s | OtherActor=%s | OtherComp=%s\n"
		"      ImpactPoint=%s | ImpactNormal=%s | Impulse=%.1f\n"
		"      Bone=%s | FaceIndex=%d | PhysMat=%s"),
		*GetNameSafe(this),
		*GetNameSafe(HitComp),
		*GetNameSafe(OtherActor),
		*GetNameSafe(OtherComp),
		*Hit.ImpactPoint.ToString(),
		*Hit.ImpactNormal.ToString(),
		NormalImpulse.Size(),
		*Hit.BoneName.ToString(),
		Hit.FaceIndex,
		Hit.PhysMaterial.IsValid() ? *Hit.PhysMaterial->GetName() : TEXT("None"));

	/** 바운스 사운드 재생 */
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastBounceSoundTime > 0.1f)
	{
		float ImpactStrength = NormalImpulse.Size();
		if (ImpactStrength > 100.0f) // 최소 충격 강도
		{
			PlayBounceSound(ImpactStrength);
			LastBounceSoundTime = CurrentTime;
		}
	}

	/** 플레이어와의 충돌 체크 */
	if (bIsThrown)
	{
		AUnderwaterCharacter* HitDiver = Cast<AUnderwaterCharacter>(OtherActor);
		if (HitDiver && HitDiver != CachedHeldBy.Get())
		{
			FVector CurrentVelocity = ProjectileMovement->Velocity;
			if (CurrentVelocity.Size() > MinVelocityForKnockback)
			{
				KnockbackPlayer(HitDiver, Hit.ImpactPoint);
			}
		}
	}
}

void AADBasketball::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OhterComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("BasketballHoop"))
	{
		FVector Velocity = ProjectileMovement->Velocity;
		if (Velocity.Z < -100.0f)
		{
			/** 골대에 공을 넣었을 때 이펙트? */
			ScoreBasket(); 
		}
	}
}

void AADBasketball::OnRep_HeldBy()
{
	if (HeldBy)
	{
		PlayBasketballAnimation(HeldBy);
		SetReplicateMovement(false);

		CollisionSphere->SetSimulatePhysics(false);
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ProjectileMovement->SetActive(false);

		if (HeldBy->GetMesh())
		{
			AttachToComponent(
				HeldBy->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				BasketballSocketName
			);
		}
		if (UADInteractionComponent* InteractionComp = HeldBy->GetInteractionComponent())
		{
			InteractionComp->SetHeldItem(this);
			LOG(TEXT("[Client] Set HeldItem in OnRep_HeldBy"));
		}
	}
	else
	{
		SetReplicateMovement(true);

		if (CachedHeldBy.IsValid())
		{
			StopBasketballAnimation(CachedHeldBy.Get());
		}

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		CollisionSphere->SetSimulatePhysics(true);
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ProjectileMovement->SetActive(true);
	}
	LOG(TEXT("[CLIENT] OnRep_HeldBy called - HeldBy: %s"),
		HeldBy ? *HeldBy->GetName() : TEXT("nullptr"));
}

void AADBasketball::OnRep_bIsThrown()
{
	if (bIsThrown)
	{
		ProjectileMovement->SetActive(true);
	}
}

void AADBasketball::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADBasketball, HeldBy);
	DOREPLIFETIME(AADBasketball, bIsThrown);
}

void AADBasketball::EnablePickupAfterThrow()
{
	bIsThrown = false;
	CachedHeldBy = nullptr;
	LOG(TEXT("Basketball can be picked up again"));
}

void AADBasketball::KnockbackPlayer(AUnderwaterCharacter* TargetPlayer, const FVector& ImpactPoint)
{
	if (!TargetPlayer || !HasAuthority())
		return;
	
	FVector KnockbackDirection = (TargetPlayer->GetActorLocation() - ImpactPoint).GetSafeNormal();
	KnockbackDirection.Z = KnockbackZDirection;
	KnockbackDirection.Normalize();

	if (UCharacterMovementComponent* MovementComp = TargetPlayer->GetCharacterMovement())
	{
		FVector Impulse = KnockbackDirection * KnockbackForce;
		MovementComp->AddImpulse(Impulse, true);

		LOG(TEXT("Player %s knocked back by basketball"), *TargetPlayer->GetName());
	}
}

void AADBasketball::ScoreBasket()
{
	/** 골대에 공을 넣었을 때 이펙트 같은 것 -> 나중에 기획 물어보기 */
	LOG(TEXT("Basketball scored!"));
}

void AADBasketball::PlayBounceSound(float ImpactStrength)
{
	float Volume = FMath::Clamp(ImpactStrength / 1000.0f, 0.1f, 1.0f);

	if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
	{
		SoundSubsystem->PlayAt(ESFX::BasketballBounce, GetActorLocation(), Volume);
	}
}

void AADBasketball::ClearOwnerIgnore()
{
	if (AActor* PrevOwner = CachedHeldBy.Get())
	{
		CollisionSphere->IgnoreActorWhenMoving(PrevOwner, false);
		CollisionSphere->MoveIgnoreActors.Remove(PrevOwner);
	}
}

FVector AADBasketball::CalculateThrowVelocity() const
{
	if (!CachedHeldBy.IsValid())
		return FVector::ZeroVector;

	AUnderwaterCharacter* Diver = CachedHeldBy.Get();

	FVector CameraLocation;
	FRotator CameraRotation;
	Diver->GetActorEyesViewPoint(CameraLocation, CameraRotation);
	FVector CameraForward = CameraRotation.Vector();

	// 라인 트레이스로 타겟 찾기
	FVector TraceEnd = CameraLocation + (CameraForward * TraceDistance);
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Diver);
	QueryParams.AddIgnoredActor(this);

	FVector TargetPoint;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd,
		ECC_Visibility, QueryParams))
	{
		TargetPoint = HitResult.Location;
	}
	else
	{
		TargetPoint = TraceEnd;
	}

	// 공 위치에서 타겟으로의 방향
	FVector BallLocation = GetActorLocation();
	FVector ThrowDirection = (TargetPoint - BallLocation).GetSafeNormal();

	float Distance = FVector::Dist(BallLocation, TargetPoint);
	// ⭐ 거리에 비례한 위쪽 힘 추가 (멀수록 더 높이 던짐)
	float DistanceFactor = FMath::Clamp(Distance / 1000.0f, 0.3f, 2.0f); // 거리 정규화
	float AdaptiveUpwardForce = ThrowUpwardForce * DistanceFactor;

	FVector Velocity = ThrowDirection * ThrowForce;
	Velocity.Z += AdaptiveUpwardForce;

	return Velocity;
}

void AADBasketball::PlayBasketballAnimation(AUnderwaterCharacter* Character)
{
	if (!Character || !BasketballHoldMontage)
	{
		LOG(TEXT("Cannot play basketball animation: Character or Montage is null"));
		return;
	}
	FAnimSyncState SyncState;
	SyncState.bEnableRightHandIK = true;
	SyncState.bEnableLeftHandIK = false;
	SyncState.bEnableFootIK = true;
	SyncState.bIsStrafing = false;

	Character->M_StopAllMontagesOnBothMesh(0.f);
	Character->M_PlayMontageOnBothMesh(BasketballHoldMontage, 1.0f, NAME_None, SyncState);

	LOG(TEXT("Basketball hold animation started"));
}

void AADBasketball::StopBasketballAnimation(AUnderwaterCharacter* Character)
{
	if (!Character)
	{
		LOG(TEXT("Cannot stop basketball animation: Character is null"));
		return;
	}
	Character->M_StopAllMontagesOnBothMesh(0.2f);

	LOG(TEXT("Basketball hold animation stopped"));
}

USoundSubsystem* AADBasketball::GetSoundSubsystem()
{
	if (!SoundSubsystemWeakPtr.IsValid())
	{
		if (UADGameInstance* GameInstance = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
		{
			SoundSubsystemWeakPtr = GameInstance->GetSubsystem<USoundSubsystem>();
		}
	}

	return SoundSubsystemWeakPtr.Get();
}
