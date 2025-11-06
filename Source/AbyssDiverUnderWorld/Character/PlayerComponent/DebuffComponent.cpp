// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerComponent/DebuffComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DebuffComponent.h"

UDebuffComponent::UDebuffComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UDebuffComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PoisonDebuffTimerHandle);
		World->GetTimerManager().ClearTimer(PoisonClearTimerHandle); 
	}
	Super::EndPlay(EndPlayReason);
} 

void UDebuffComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UDebuffComponent, CurrentDebuffState);
}

void UDebuffComponent::SetDebuffState(FDebuffInfo NewStateInfo)
{
	if (CurrentDebuffState != NewStateInfo.DebuffType)
	{
		CurrentDebuffState = NewStateInfo.DebuffType;
		ApplyDebuff(NewStateInfo);
	}
}

void UDebuffComponent::ApplyDebuff(FDebuffInfo NewStateInfo)
{
	float PoisonTickInterval = 1.0f;
	switch (NewStateInfo.DebuffType)
	{
	case EDebuffType::None:
		// Clear all debuffs
		break;
	case EDebuffType::Poison:
		// Apply poison effect
		GetWorld()->GetTimerManager().SetTimer(PoisonDebuffTimerHandle, [this, NewStateInfo]()
			{
				if (IsValid(this))
				{
					if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
					{
						UGameplayStatics::ApplyDamage(Owner, NewStateInfo.Damage, nullptr, nullptr, nullptr);
						UE_LOG(LogTemp, Warning, TEXT("PoisonDamage"));
					}
				}
			}, PoisonTickInterval, true);

		GetWorld()->GetTimerManager().SetTimerForNextTick([this]() //안전한 실행 순서를 보장하기 위해 다음 틱(프레임)에 실행되도록 예약하는 함수
			{
				float PoisonDuration = 3.0f;
				GetWorld()->GetTimerManager().SetTimer(PoisonClearTimerHandle, FTimerDelegate::CreateLambda([this]()
					{
						GetWorld()->GetTimerManager().ClearTimer(PoisonDebuffTimerHandle);
						CurrentDebuffState = EDebuffType::None;
					}), PoisonDuration, false);
			});

		break;
	default:
		break;
	}
}

