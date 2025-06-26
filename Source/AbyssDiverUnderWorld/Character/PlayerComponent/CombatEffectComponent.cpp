// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatEffectComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "ShieldComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Character/UnderwaterCharacter.h"
#include "DataRow/SoundDataRow/SFXDataRow.h"
#include "Framework/ADPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SoundSubsystem.h"

UCombatEffectComponent::UCombatEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	HitBlackoutDuration = 0.1f;
	HitFadeInDuration = 0.2f;

	ShieldBrokenSound = ESFX::ShieldBroken;
	ShieldHitSound = ESFX::ShieldHit;
	ShieldUseSound = ESFX::UseShield;	
	DamageTakenSound = ESFX::DamageTaken;
}

void UCombatEffectComponent::C_PlayShieldUseEffect_Implementation()
{
	UE_LOG(LogTemp, Display, TEXT("Play Shield Hit Effect"));
	if (ShieldHitWidget && ShieldUseAnimation && !ShieldHitWidget->IsAnyAnimationPlaying())
	{
		ShieldHitWidget->PlayAnimation(ShieldUseAnimation);
	}
	if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
	{
		SoundSubsystem->Play2D(ShieldUseSound);
	}
}

void UCombatEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetOwner()))
	{
		OwnerCharacter = UnderwaterCharacter;
		BindDelegate(UnderwaterCharacter);
	}
}

void UCombatEffectComponent::BindDelegate(AUnderwaterCharacter* UnderwaterCharacter)
{
	if (UShieldComponent* ShieldComponent = UnderwaterCharacter->GetShieldComponent())
	{
		ShieldComponent->OnShieldBrokenDelegate.AddDynamic(this, &UCombatEffectComponent::OnShieldBroken);
		if (ShieldBrokenEffect)
		{
			ShieldBrokenEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				ShieldBrokenEffect,
				GetOwner()->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::Type::SnapToTarget,
				false,
				false
			);
		}

		ShieldComponent->OnShieldValueChangedDelegate.AddDynamic(this, &UCombatEffectComponent::OnShieldValueChanged);
		if (ShieldHitWidgetClass)
		{
			ShieldHitWidget = CreateWidget<UUserWidget>(GetWorld(), ShieldHitWidgetClass);
			if (ShieldHitWidget)
			{
				ShieldHitWidget->AddToViewport();
				ShieldHitAnimation = FindAnimationByName(ShieldHitWidget, ShieldHitAnimationName);
				ShieldUseAnimation = FindAnimationByName(ShieldHitWidget, ShieldUseAnimationName);
			}
			else
			{
				UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("Failed to create Shield Hit Widget: %s"), *ShieldHitWidgetClass->GetName());
			}
		}
	}

	// OnTakeAnyDamage를 사용할려면 구조를 바꾸어야 하기 때문에 UnderwaterCharacter에서 구현한 OnDamageTakenDelegate를 이용한다.
	// 다른 이벤트들과 다르게 OnDamageTakenDelegate는 Serer에서만 작동한다. Client RPC를 이용해야 해서 전달해야 한다.
	UnderwaterCharacter->OnDamageTakenDelegate.AddUniqueDynamic(this, &UCombatEffectComponent::OnDamageTaken);
}

void UCombatEffectComponent::OnShieldBroken()
{
	if (OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}
	
	if (ShieldBrokenEffectComponent && !ShieldBrokenEffectComponent->IsActive())
	{
		ShieldBrokenEffectComponent->Activate(true);

		
		if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
		{
			SoundSubsystem->Play2D(ShieldBrokenSound);
		}
	}
}

void UCombatEffectComponent::OnShieldValueChanged(float OldShieldValue, float NewShieldValue)
{
	if (OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}
	
	UE_LOG(LogTemp,Display, TEXT("Shield Value Changed: Old = %f, New = %f"), OldShieldValue, NewShieldValue);
	if (NewShieldValue < OldShieldValue && NewShieldValue > 0.0f)
	{
		PlayShieldHitEffect();
	}
	else if (NewShieldValue > OldShieldValue)
	{
		// 실드가 회복되었을 때의 처리 (필요시 구현)
	}
}

void UCombatEffectComponent::OnDamageTaken(float DamageAmount, float CurrentHealth)
{
	if (DamageAmount <= 0.0f || !OwnerCharacter)
	{
		return;
	}

	AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetOwner());
	if (!UnderwaterCharacter)
	{
		return;
	}

	AADPlayerController* PlayerController = Cast<AADPlayerController>(GetOwner()->GetInstigatorController());
	if (!PlayerController)
	{
		return;
	}

	if (UnderwaterCharacter->IsNormal() && !UnderwaterCharacter->IsCaptured())
	{
		PlayerController->C_StartCameraBlink(
			FColor::Black,
			FVector2D(0.0f, 1.0f),
			0.0f,
			HitBlackoutDuration,
			HitFadeInDuration
		);

		if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
		{
			SoundSubsystem->Play2D(DamageTakenSound);
		}
	}
}

void UCombatEffectComponent::PlayShieldHitEffect()
{
	if (OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}
	
	UE_LOG(LogTemp,Display, TEXT("Play Shield Hit Effect"));
	if (ShieldHitWidget && ShieldHitAnimation && !ShieldHitWidget->IsAnyAnimationPlaying())
	{
		ShieldHitWidget->PlayAnimation(ShieldHitAnimation);
	}

	if (USoundSubsystem* SoundSubsystem = GetSoundSubsystem())
	{
		SoundSubsystem->Play2D(ShieldHitSound);
	}
}

UWidgetAnimation* UCombatEffectComponent::FindAnimationByName(UUserWidget* Widget,
	const FName& AnimationName)
{
	if (!Widget)
	{
		return nullptr;
	}

	const UClass* WidgetClass = Widget->GetClass();
	for (TFieldIterator<FObjectProperty> PropIt(WidgetClass, EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
	{
		FObjectProperty* ObjectProp = *PropIt;
		if (ObjectProp && ObjectProp->PropertyClass == UWidgetAnimation::StaticClass())
		{
			UWidgetAnimation* Animation = Cast<UWidgetAnimation>(ObjectProp->GetObjectPropertyValue_InContainer(Widget));
			// Blueprint Animation은 _INST로 끝나므로 프로퍼티 이름 자체를 비교한다.
			if (Animation && ObjectProp->GetFName() == AnimationName)
			{
				return Animation;
			}
		}
	}
	
	return  nullptr;
}

UUserWidget* UCombatEffectComponent::GetShieldHitWidget()
{
	return ShieldHitWidget;
}

class USoundSubsystem* UCombatEffectComponent::GetSoundSubsystem()
{
	if (!SoundSubsystemWeakPtr.IsValid())
	{
		if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			SoundSubsystemWeakPtr = GameInstance->GetSubsystem<USoundSubsystem>();
		}
		else
		{
			UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("Failed to get SoundSubsystem from GameInstance"));
		}
	}

	return SoundSubsystemWeakPtr.Get();
}
