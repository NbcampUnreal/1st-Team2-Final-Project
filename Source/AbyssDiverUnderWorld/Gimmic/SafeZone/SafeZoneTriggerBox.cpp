#include "Gimmic/SafeZone/SafeZoneTriggerBox.h"
#include "AbyssDiverUnderWorld.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"

ASafeZoneTriggerBox::ASafeZoneTriggerBox()
{
	// Character의 Interaction Volume은 무시하고 Character Capsule Component와만 충돌
	if (UShapeComponent* TriggerComponent = GetCollisionComponent())
	{
		TriggerComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	}
}

void ASafeZoneTriggerBox::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Character)) return;

	OverlappingCharacters.Add(Character);
	if (IsActorInZone(Character))
	{
		Character->SetHideInSeaweed(true);
	}

	if (!GetWorldTimerManager().IsTimerActive(UpdateTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			UpdateTimerHandle,
			this,
			&ASafeZoneTriggerBox::UpdateCharactersInZone,
			UpdateInterval,
			true
		);
	}
}

void ASafeZoneTriggerBox::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(OtherActor);
	OverlappingCharacters.Remove(Character);

	if (IsValid(Character))
	{
		Character->SetHideInSeaweed(false);
	}

	if (OverlappingCharacters.IsEmpty() && GetWorldTimerManager().IsTimerActive(UpdateTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(UpdateTimerHandle);
	}
}

bool ASafeZoneTriggerBox::IsActorInZone(AUnderwaterCharacter* Character)
{
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	if (!Capsule) return false;

	UShapeComponent* TriggerComponent = GetCollisionComponent();
	if (!TriggerComponent) return false;

	FVector CapsuleLocation = Capsule->GetComponentLocation();
	float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	float CapsuleRadius = Capsule->GetScaledCapsuleRadius();

	// Capsule의 Top, Bottom의 World Location
	FVector Top = CapsuleLocation + FVector::UpVector * CapsuleHalfHeight;
	FVector Bottom = CapsuleLocation + FVector::DownVector * CapsuleHalfHeight;

	FBox VolumeBox = TriggerComponent->Bounds.GetBox();

	// Capsule의 Top, Bottom을 기준으로 반지름이 VolumeBox 내부에 있는지 판단
	auto IsPointInBox = [&](const FVector& Point) -> bool
	{
		return VolumeBox.IsInside(Point + FVector::ForwardVector * CapsuleRadius) &&
			   VolumeBox.IsInside(Point + FVector::BackwardVector * CapsuleRadius) &&
			   VolumeBox.IsInside(Point + FVector::RightVector * CapsuleRadius) &&
			   VolumeBox.IsInside(Point + FVector::LeftVector * CapsuleRadius);
	};

	return IsPointInBox(Top) && IsPointInBox(Bottom);
}

void ASafeZoneTriggerBox::UpdateCharactersInZone()
{
	for (auto It = OverlappingCharacters.CreateIterator(); It; ++It)
	{
		TWeakObjectPtr<AUnderwaterCharacter> CharacterPtr = *It;
		if (!CharacterPtr.IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		AUnderwaterCharacter* Character = CharacterPtr.Get();
		const bool bInZone = IsActorInZone(Character);
		Character->SetHideInSeaweed(bInZone);
	}

	if (OverlappingCharacters.IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(UpdateTimerHandle);
	}
}
