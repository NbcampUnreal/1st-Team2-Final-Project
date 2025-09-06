#include "Monster/Boss/AlienShark/AlienShark.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/CapsuleComponent.h"
#include "Container/BlackboardKeys.h"

AAlienShark::AAlienShark()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BiteCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Bite Collision"));
	BiteCollision->SetupAttachment(GetMesh(), TEXT("BiteSocket"));
	BiteCollision->SetCapsuleHalfHeight(80.0f);
	BiteCollision->SetCapsuleRadius(80.0f);
	BiteCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BiteCollision->ComponentTags.Add(TEXT("Bite Collision"));
}

void AAlienShark::BeginPlay()
{
	Super::BeginPlay();

	BiteCollision->OnComponentBeginOverlap.AddDynamic(this, &AAlienShark::OnMeshOverlapBegin);
}

void AAlienShark::NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor)
{
	// AlienShark는 한 번 공격 이후에 일정 시간 동안 Chase가 불가능 하므로 Chase 불가능 상태이면 return
	if (IsValid(AIController) == false)
	{
		LOGV(Error, TEXT("NotifyLightExposure: AIController is not valid"));
		return;
	}

	bool bCanChase = AIController->GetBlackboardComponent()->GetValueAsBool(BlackboardKeys::AlienShark::bCanChaseKey);
	if (!bCanChase) return;

	Super::NotifyLightExposure(DeltaTime, TotalExposedTime, PlayerLocation, PlayerActor);
}
