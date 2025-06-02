// Fill out your copyright notice in the Description page of Project Settings.


#include "LanternComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/SpringArmComponent.h"
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
}

void ULanternComponent::RequestToggleLanternLight()
{
	UE_LOG(LogTemp,Display, TEXT("Request Toggle Lantern Light : %s"), bIsLanternOn ? TEXT("On") : TEXT("Off"));
	if (GetOwnerRole() == ROLE_Authority)
	{
		bIsLanternOn = !bIsLanternOn;
		OnRep_bIsLanternOn();
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
	UE_LOG(LogTemp,Display, TEXT("Spawn Light"));
	
	LanternLightComponent = NewObject<USpotLightComponent>(this, USpotLightComponent::StaticClass(), TEXT("LanternLightComponent"));
	AUnderwaterCharacter* CharacterOwner = Cast<AUnderwaterCharacter>(GetOwner());
	
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
}

void ULanternComponent::S_ToggleLanternLight_Implementation()
{
	RequestToggleLanternLight();
}
