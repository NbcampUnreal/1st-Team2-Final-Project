#include "Monster/EyeStalker/EyeStalker.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Components/AquaticMovementComponent.h"
#include "Monster/Components/TickControlComponent.h"

#include "Character/UnderwaterCharacter.h"
#include "Framework/ADPlayerController.h"
#include "Monster/Effect/PostProcessSettingComponent.h"

AEyeStalker::AEyeStalker()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEyeStalker::BeginPlay()
{
	Super::BeginPlay();

	if (AquaticMovementComponent)
	{
		TickControlComponent->UnregisterComponent(AquaticMovementComponent);
	}
}

float AEyeStalker::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	OnEyeStalkerDamaged();

	AMonsterAIController* AIC = Cast<AMonsterAIController>(GetController());
	if (AIC == nullptr)
	{
		LOGV(Error, TEXT("AIController Is not valid"));
		return Damage;
	}

	AActor* InstigatorPlayer = IsValid(EventInstigator) ? EventInstigator->GetPawn() : DamageCauser;
	if (IsValid(InstigatorPlayer)== false)
	{
		LOGV(Error, TEXT("Instigator is not valid"));
		return Damage;
	}

	// 시야 범위 밖일 경우 데미지를 받았더라도 어그로 끌리지 않음.
	if ((InstigatorPlayer->GetActorLocation() - GetActorLocation()).Length() >= AIC->GetSightRadius())
	{
		RemoveDetection(DamageCauser);
	}

	return Damage;
}

void AEyeStalker::NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor)
{
	// EyeStalker는 빛에 반응 하지 않음
}

void AEyeStalker::AddDetection(AActor* Actor)
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
	}
	else return;

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

void AEyeStalker::RemoveDetection(AActor* Actor)
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
	const bool bHasTarget = (TargetPlayer.Get() == Actor);
	if (bHasTarget)
	{
		// Task Node에서는 Remove Detection으로 빠지기 때문에 잃어버린 Target에 대해서 처리할 수가 없기 때문에 여기서 처리 
		Player->SetIsAttackedByEyeStalker(false);
		if (UPostProcessSettingComponent* PostProcessSettingComponent = Player->GetPostProcessSettingComponent())
		{
			PostProcessSettingComponent->C_DeactivateVignetteEffect();
		}
		if (AADPlayerController* PC = Player->GetController<AADPlayerController>())
		{
			PC->C_SetRadialBlurEffect(false);
		}
		
		// TSet 순회하여 요소(Player)가 남아있으면 첫번째 플레이어를 TargetPlayer로 지정
		AUnderwaterCharacter* DetectedPlayer = nullptr;
		for (const TWeakObjectPtr<AActor>& Elem : DetectedPlayers)
		{
			if (AActor* NewTarget = Elem.Get())
			{
				DetectedPlayer = Cast<AUnderwaterCharacter>(NewTarget);
				break;
			}
		}
		
		TargetPlayer = DetectedPlayer;
		SetTarget(DetectedPlayer);
	}
}

void AEyeStalker::ReceiveKnockback(const FVector& Force)
{
	// 넉백 당하지 않음.
}

void AEyeStalker::M_SetEyeOpenness_Implementation(float Openness)
{
	SetEyeOpenness(Openness);
}

void AEyeStalker::M_SetTargetPlayer_Implementation(AUnderwaterCharacter* Player)
{
	SetTargetPlayer(Player);
}

void AEyeStalker::M_SetDetectedState_Implementation(bool bDetected)
{
	SetDetectedState(bDetected);
}
