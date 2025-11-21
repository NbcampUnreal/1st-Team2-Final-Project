// Fill out your copyright notice in the Description page of Project Settings.


#include "LanternComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "Monster/Monster.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"

ULanternComponent::ULanternComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
	bIsLanternOn = false;
	LanternForwardOffset = 10.0f; // 라이트가 캐릭터 앞에 위치하도록 설정

	LightAngle = 35.0f;
	Intensity = 150000.0f;
	RemoveDelayTime = 7.0f;
}

void ULanternComponent::BeginPlay()
{
	Super::BeginPlay();
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

	if (GetOwnerRole() != ROLE_Authority || !LanternLightComponent) return;
	
	TArray<AActor*> Candidates;

	if (bIsLanternOn)
	{
		LightDetectionComponent->GetOverlappingActors(Candidates, AMonster::StaticClass());
	}

	UpdateExposureTimes(Candidates, DeltaTime);
}

void ULanternComponent::RequestToggleLanternLight()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		SetLanternLightEnabled(!bIsLanternOn);
	}
	else
	{
		S_ToggleLanternLight();
	}
}

void ULanternComponent::OnRep_bIsLanternOn()
{
	if (LanternLightComponent)
	{
		LanternLightComponent->SetVisibility(bIsLanternOn);
	}
}

void ULanternComponent::UpdateDetectionShape(float ConeHeight, float ConeAngle)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	const FVector Direction = LanternLightComponent->GetForwardVector().GetSafeNormal();
	const FVector Origin = LanternLightComponent->GetComponentLocation();
		
	// 원뿔이 높이 : 삼각형의 밑변 : Capsule Height : AttenuationRadius
	// 원뿔의 반지름 : 삼각형의 높이 : Capsule Radius : AttenuationRadius * tan(OuterConeAngle)
	const float ConeRadius = ConeHeight * FMath::Tan(FMath::DegreesToRadians(ConeAngle));
	const float CapsuleHalfHeight = ConeHeight * 0.5f + ConeRadius; // 캡슐의 반지름 + 캡슐의 높이 절반

	// 캡슐의 중심 위치는 라이트 위치에서 방향으로 ConeHeight의 절반만큼 이동한 위치
	const FVector CapsuleCenter = Origin + Direction * ConeHeight * 0.5f;
	// Z축이 Direction 방향이 되도록 회전 행렬을 생성
	// const FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(Direction).ToQuat();
	const FQuat CapsuleRotation = FQuat::FindBetweenVectors(FVector::UpVector, Direction);
		
	LightDetectionComponent->InitCapsuleSize(ConeRadius, CapsuleHalfHeight);
	LightDetectionComponent->SetWorldLocationAndRotation(CapsuleCenter, CapsuleRotation);
}

void ULanternComponent::SpawnLight(USceneComponent* AttachToComponent, const float NewLightLength)
{
	// 이미 존재하면 더 이상 생성하지 않는다.
	if (LanternLightComponent)
	{
		return;
	}
	
	LightLength = NewLightLength;
	
	LanternLightComponent = NewObject<USpotLightComponent>(this, USpotLightComponent::StaticClass(), TEXT("LanternLightComponent"));
	LanternLightComponent->SetOuterConeAngle(LightAngle);
	LanternLightComponent->SetAttenuationRadius(NewLightLength); // 거리에 영향을 준다.
	LanternLightComponent->SetIntensity(Intensity);
	LanternLightComponent->SetVisibility(bIsLanternOn);
	LanternLightComponent->RegisterComponent();
	LanternLightComponent->SetLightingChannels(true, false, true);
	LanternLightComponent->AttachToComponent(AttachToComponent,FAttachmentTransformRules::SnapToTargetIncludingScale);

	const FVector ForwardVector = GetOwner()->GetActorForwardVector();
	const FRotator LightRotation = ForwardVector.Rotation();
	const FVector LightLocation = AttachToComponent->GetComponentLocation() + (ForwardVector * LanternForwardOffset);
	LanternLightComponent->SetWorldLocationAndRotation(LightLocation, LightRotation);

	if (GetOwnerRole() == ROLE_Authority)
	{
		LightDetectionComponent = NewObject<UCapsuleComponent>(this, UCapsuleComponent::StaticClass(), TEXT("LanternDetectionComponent"));
		LightDetectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LightDetectionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		// ECC_GameTraceChannel3 == Monster (ProjectSetting)
		LightDetectionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
		LightDetectionComponent->SetGenerateOverlapEvents(true);
		LightDetectionComponent->AttachToComponent(LanternLightComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		LightDetectionComponent->RegisterComponent();

		UpdateDetectionShape(LanternLightComponent->AttenuationRadius, LanternLightComponent->OuterConeAngle);
	}
}

void ULanternComponent::SetLightLength(float NewLightLength)
{
	if (LightLength == NewLightLength)
	{
		return; // 길이가 변경되지 않았다면 아무것도 하지 않음
	}

	LightLength = NewLightLength;
	if (LanternLightComponent)
	{
		LanternLightComponent->SetAttenuationRadius(LightLength);
		UpdateDetectionShape(LightLength, LanternLightComponent->OuterConeAngle);
	}
	else
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("LanternLightComponent is not valid. Should Spawn Light first."));
	}
}

void ULanternComponent::SetLanternLightEnabled(const bool bEnable)
{
	if (bIsLanternOn == bEnable)
	{
		return;
	}

	bIsLanternOn = bEnable;
	OnRep_bIsLanternOn();

	if (GetOwnerRole() == ROLE_Authority && LightDetectionComponent)
	{
		LightDetectionComponent->SetCollisionEnabled(
			bIsLanternOn ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision
		);
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

	if (bIsLanternOn)
	{
		ProcessExposure(OverlappedActors, DeltaTime);
	}
	else // Lantern Off
	{
		AccumulateUnexposeTime(DeltaTime);
	}

	HandleUnexposedMonsters(DeltaTime);
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
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		ConeOrigin,
		TargetActor->GetActorLocation(),
		ECC_Visibility,
		Params
	);

	// 몬스터와 라이트 사이에 장애물이 없고, 몬스터가 라이트의 범위 안에 있다면 노출
	return bHit && HitResult.GetActor()->IsA(AMonster::StaticClass());
}

void ULanternComponent::ProcessExposure(const TArray<AActor*>& OverlappedActors, float DeltaTime)
{
	const FVector LightLocation = LanternLightComponent->GetComponentLocation();
	const FVector LightForward = LanternLightComponent->GetForwardVector();
	const float LightConeAngleDegree = LanternLightComponent->OuterConeAngle;
	const float LightRange = LanternLightComponent->AttenuationRadius;

	for (AActor* Actor : OverlappedActors)
	{
		AMonster* Monster = Cast<AMonster>(Actor);
		if (!IsValid(Monster)) continue;

		if (HasActorExposedByLight(Actor, LightLocation, LightForward, LightConeAngleDegree, LightRange))
		{
			// Accumulating Exposure time
			float& ExposureTime = MonsterExposeTimeMap.FindOrAdd(Monster);
			ExposureTime += DeltaTime;
			Monster->NotifyLightExposure(DeltaTime, ExposureTime, GetOwner()->GetActorLocation(), GetOwner());

			// Reset Unexposure time
			MonsterUnexposeTimeMap.Remove(Monster);
		}
		// Cone 바깥이라면 노출 시간을 초기화
		else
		{
			// Reset exposure time if out of view
			if (MonsterExposeTimeMap.Contains(Monster))
			{
				MonsterExposeTimeMap[Monster] = 0.0f;
			}

			// Accumulating Unexposure time
			float& UnexposedTime = MonsterUnexposeTimeMap.FindOrAdd(Monster);
			UnexposedTime += DeltaTime;
		}
	}
}

void ULanternComponent::AccumulateUnexposeTime(float DeltaTime)
{
	for (const auto& Pair : MonsterExposeTimeMap)
	{
		AMonster* Monster = Pair.Key;
		if (!IsValid(Monster)) continue;

		float& UnexposeTime = MonsterUnexposeTimeMap.FindOrAdd(Monster);
		UnexposeTime += DeltaTime;
	}
}

void ULanternComponent::HandleUnexposedMonsters(float DeltaTime)
{
	// Checking for undetect conditions
	for (auto It = MonsterUnexposeTimeMap.CreateIterator(); It; ++It)
	{
		AMonster* Monster = It.Key();
		float UnexposeTime = It.Value();

		// Remove if dead or invalid
		if (!IsValid(Monster))
		{
			It.RemoveCurrent();
			MonsterExposeTimeMap.Remove(Monster);
			continue;
		}

		if (UnexposeTime >= RemoveDelayTime)
		{
			It.RemoveCurrent();
			MonsterExposeTimeMap.Remove(Monster);
		}
	}
}
