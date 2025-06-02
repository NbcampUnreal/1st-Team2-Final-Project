// Fill out your copyright notice in the Description page of Project Settings.


#include "LanternComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Monster/Monster.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
ULanternComponent::ULanternComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
	bIsLanternOn = false;
	LanternForwardOffset = 10.0f; // 라이트가 캐릭터 앞에 위치하도록 설정
}


// Called when the game starts
void ULanternComponent::BeginPlay()
{
	Super::BeginPlay();

	SpawnLight();
}

void ULanternComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULanternComponent, bIsLanternOn);
}


// Called every frame
void ULanternComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_Authority && bIsLanternOn && LightDetectionComponent)
	{
		TArray<AActor*> Candidates;
		LightDetectionComponent->GetOverlappingActors(Candidates, AMonster::StaticClass());
		UpdateExposureTimes(Candidates, DeltaTime);
	}
}

void ULanternComponent::RequestToggleLanternLight()
{
	UE_LOG(LogTemp,Display, TEXT("Request Toggle Lantern Light : %s"), bIsLanternOn ? TEXT("On") : TEXT("Off"));
	if (GetOwnerRole() == ROLE_Authority)
	{
		bIsLanternOn = !bIsLanternOn;
		OnRep_bIsLanternOn();
		
		if (LightDetectionComponent)
		{
			LightDetectionComponent->SetCollisionEnabled(
				bIsLanternOn ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision
			);
		}
		
		if (!bIsLanternOn)
		{
			MonsterExposeTimeMap.Empty();
		}
	}
	else
	{
		S_ToggleLanternLight();
	}
}

void ULanternComponent::OnRep_bIsLanternOn()
{
	LanternLightComponent->SetVisibility(bIsLanternOn);
}

void ULanternComponent::SpawnLight()
{
	AUnderwaterCharacter* CharacterOwner = Cast<AUnderwaterCharacter>(GetOwner());
	
	LanternLightComponent = NewObject<USpotLightComponent>(this, USpotLightComponent::StaticClass(), TEXT("LanternLightComponent"));
	LanternLightComponent->SetOuterConeAngle(25.0f);
	LanternLightComponent->SetAttenuationRadius(2000.0f); // 거리에 영향을 준다.
	LanternLightComponent->SetIntensity(200000.0f);
	LanternLightComponent->SetVisibility(bIsLanternOn);
	LanternLightComponent->RegisterComponent();
	LanternLightComponent->AttachToComponent(CharacterOwner->GetMesh1PSpringArm(),FAttachmentTransformRules::SnapToTargetIncludingScale);

	const FVector ForwardVector = CharacterOwner->GetActorForwardVector();
	const FRotator LightRotation = ForwardVector.Rotation();
	const FVector LightLocation = CharacterOwner->GetMesh1PSpringArm()->GetComponentLocation() + (ForwardVector * LanternForwardOffset);
	LanternLightComponent->SetWorldLocationAndRotation(LightLocation, LightRotation);

	if (GetOwnerRole() == ROLE_Authority)
	{
		LightDetectionComponent = NewObject<UCapsuleComponent>(this, UCapsuleComponent::StaticClass(), TEXT("LanternDetectionComponent"));
		LightDetectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LightDetectionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		LightDetectionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		LightDetectionComponent->SetGenerateOverlapEvents(true);
		LightDetectionComponent->AttachToComponent(LanternLightComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		LightDetectionComponent->RegisterComponent();

		const FVector Direction = LanternLightComponent->GetForwardVector().GetSafeNormal();
		const FVector Origin = LanternLightComponent->GetComponentLocation();
		
		// 원뿔이 높이 : 삼각형의 밑변 : Capsule Height : AttenuationRadius
		// 원뿔의 반지름 : 삼각형의 높이 : Capsule Radius : AttenuationRadius * tan(OuterConeAngle)
		const float ConeHeight = LanternLightComponent->AttenuationRadius;
		const float ConeRadius = ConeHeight * FMath::Tan(FMath::DegreesToRadians(LanternLightComponent->OuterConeAngle));
		const float CapsuleHalfHeight = ConeHeight * 0.5f + ConeRadius; // 캡슐의 반지름 + 캡슐의 높이 절반

		const FVector CapsuleCenter = Origin + Direction * ConeHeight * 0.5f; // 캡슐의 중심 위치는 라이트 위치에서 방향으로 ConeHeight의 절반만큼 이동한 위치
		// Z축이 Direction 방향이 되도록 회전 행렬을 생성
		// const FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(Direction).ToQuat();
		const FQuat CapsuleRotation = FQuat::FindBetweenVectors(FVector::UpVector, Direction);
		
		LightDetectionComponent->InitCapsuleSize(ConeRadius, CapsuleHalfHeight);
		LightDetectionComponent->SetWorldLocationAndRotation(CapsuleCenter, CapsuleRotation);
	}
}

void ULanternComponent::S_ToggleLanternLight_Implementation()
{
	RequestToggleLanternLight();
}

void ULanternComponent::UpdateExposureTimes(TArray<AActor*> OverlappedActors, const float DeltaTime)
{
	const FVector LightLocation = LanternLightComponent->GetComponentLocation();
	const FVector LightForward = LanternLightComponent->GetForwardVector();
	const float LightConeAngleDegree = LanternLightComponent->OuterConeAngle;
	const float LightRange = LanternLightComponent->AttenuationRadius;
	
	for (AActor* Actor: OverlappedActors)
	{
		AMonster* Monster = Cast<AMonster>(Actor);
		if (!IsValid(Monster)) continue;
		
		if (HasActorExposedByLight(Actor, LightLocation, LightForward, LightConeAngleDegree, LightRange))
		{
			float& ExposureTime = MonsterExposeTimeMap.FindOrAdd(Monster);
			ExposureTime += DeltaTime;
			// Notify Monster
		}
		// Cone 바깥이라면 노출 시간을 초기화
		else if (MonsterExposeTimeMap.Contains(Monster))
		{
			MonsterExposeTimeMap[Monster] = 0.0f; // 노출 시간이 초기화됨
		}
	}

	// 노출된 Actor가 더 이상 OverlappedActors에 없거나 유효하지 않으면 제거
	for (auto It = MonsterExposeTimeMap.CreateIterator(); It; ++It)
	{
		if (!IsValid(It.Key()) || !OverlappedActors.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}
}

bool ULanternComponent::HasActorExposedByLight(const AActor* TargetActor, const FVector& ConeOrigin, const FVector& ConeDirection,
	float ConeAngle, float ConeHeight) const
{
	FVector DirectionToMonster = TargetActor->GetActorLocation() - ConeOrigin;
	const float DistanceToMonster = DirectionToMonster.Size();
	DirectionToMonster.Normalize();
	
	float DotProduct = FVector::DotProduct(ConeDirection, DirectionToMonster);
	float AngleToMonster = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
	if (AngleToMonster > ConeAngle || DistanceToMonster > ConeHeight)
	{
		// 몬스터가 라이트의 범위 밖에 있거나, 라이트의 각도 범위를 벗어남
		return false;
	}

	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		ConeOrigin,
		TargetActor->GetActorLocation(),
		ECC_Visibility
	);

	// 몬스터와 라이트 사이에 장애물이 없고, 몬스터가 라이트의 범위 안에 있다면 노출
	return bHit && HitResult.GetActor() == TargetActor;
}
