// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatEffectComponent.h"

#include "MovieScene.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "ShieldComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Character/UnderwaterCharacter.h"
#include "Kismet/GameplayStatics.h"

UCombatEffectComponent::UCombatEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCombatEffectComponent::C_PlayShieldUseEffect_Implementation()
{
	UE_LOG(LogTemp, Display, TEXT("Play Shield Hit Effect"));
	if (ShieldHitWidget && ShieldUseAnimation && !ShieldHitWidget->IsAnyAnimationPlaying())
	{
		ShieldHitWidget->PlayAnimation(ShieldUseAnimation);
	}
	//if (ShieldUseSound)
	//{
		// @ToDo: SoundSubsystem을 사용하여 사운드 재생
		// @ToDo: SoundSubsystem의 Play ID 기능을 이용해서 현재 재생 중이면 소리를 재생하지 않도록 수정
		//UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShieldUseSound, GetOwner()->GetActorLocation());
	//}
}

void UCombatEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(GetOwner()))
	{
		// Global Effect

		if (UnderwaterCharacter->IsLocallyControlled())
		{
			BindLocalEffects(UnderwaterCharacter);
		}
	}
}

void UCombatEffectComponent::BindLocalEffects(AUnderwaterCharacter* UnderwaterCharacter)
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
}

void UCombatEffectComponent::OnShieldBroken()
{
	if (ShieldBrokenEffectComponent && !ShieldBrokenEffectComponent->IsActive())
	{
		ShieldBrokenEffectComponent->Activate(true);
		if (ShieldBrokenSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShieldBrokenSound, GetOwner()->GetActorLocation());
		}
	}
}

void UCombatEffectComponent::OnShieldValueChanged(float OldShieldValue, float NewShieldValue)
{
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

void UCombatEffectComponent::PlayShieldHitEffect()
{
	UE_LOG(LogTemp,Display, TEXT("Play Shield Hit Effect"));
	if (ShieldHitWidget && ShieldHitAnimation && !ShieldHitWidget->IsAnyAnimationPlaying())
	{
		ShieldHitWidget->PlayAnimation(ShieldHitAnimation);
	}
	if (ShieldHitSound)
	{
		// @ToDo: SoundSubsystem을 사용하여 사운드 재생
		// @ToDo: SoundSubsystem의 Play ID 기능을 이용해서 현재 재생 중이면 소리를 재생하지 않도록 수정
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShieldHitSound, GetOwner()->GetActorLocation());
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
