// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraEffectComponent.h"

#include "Camera/CameraComponent.h"
#include "Character/UnderwaterCharacter.h"


// Sets default values for this component's properties
UCameraEffectComponent::UCameraEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UCameraEffectComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCameraEffectComponent::StartRadialBlurFade(bool bIsFadeIn)
{
	if (!GetOwner()) return;

	ApplyRadialBlurEffect();
	
	RadialBlurTargetAlpha = bIsFadeIn ? 1.0f : 0.0f;
	RadialBlurAlpha = 0.0f;
	// 현재 Material 의 값을 가져와서 초기값으로 설정해서 Fade In 중에 Fade Out 전환 처리
	// PreCondition: RadialBlurSpreadParam의 초기값이 0.0f로 설정되어 있어야 한다.
	if (RadialBlurMID)
	{
		RadialBlurMID->GetScalarParameterValue(RadialBlurSpreadParam, RadialBlurAlpha);
		RadialBlurTargetValue = FMath::Max(RadialBlurTargetValue, 0.001f);
		RadialBlurAlpha /= RadialBlurTargetValue;
	}

	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("StartRadialBlurFade : bIsFadeIn=%s, RadialBlurAlpha=%.2f, RadialBlurTargetAlpha=%.2f"),
		bIsFadeIn ? TEXT("True") : TEXT("False"),
		RadialBlurAlpha,
		RadialBlurTargetAlpha
	);
	
	GetWorld()->GetTimerManager().SetTimer(
		RadialBlurTimerHandle,
		this,
		&UCameraEffectComponent::UpdateRadialBlur,
		RadialBlurUpdateInterval,
		true
	);
}

void UCameraEffectComponent::ApplyRadialBlurEffect()
{
	if (!RadialBlurMaterial)
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("RadialBlurMaterial is not assigned"));
		return;
	}
	
	if (!RadialBlurMID)
	{
		RadialBlurMID = UMaterialInstanceDynamic::Create(RadialBlurMaterial, this);
		if (!RadialBlurMaterial)
		{
			UE_LOG(LogAbyssDiverCharacter, Error, TEXT("Failed to create RadialBlurMid Material Instance Dynamic"));
			return;
		}
	}

	AUnderwaterCharacter* OwnerCharacter = GetOwner<AUnderwaterCharacter>();
	UCameraComponent* ApplyCameraComponent = OwnerCharacter ? OwnerCharacter->GetFirstPersonCameraComponent() : nullptr;
	if (!ApplyCameraComponent)
	{
		UE_LOG(LogAbyssDiverCharacter, Warning, TEXT("ApplyCameraComponent is not found"));
		return;
	}

	TArray<FWeightedBlendable>& PostProcessArr = ApplyCameraComponent->PostProcessSettings.WeightedBlendables.Array;
	for (FWeightedBlendable& Blendable : PostProcessArr)
	{
		// 이미 Radial Blur가 적용되어 있을 경우 리턴
		if (Blendable.Object == RadialBlurMID)
		{
			return;
		}
	}
	PostProcessArr.Add(FWeightedBlendable(1.0f, RadialBlurMID));
}

void UCameraEffectComponent::RemoveRadialBlur()
{
	UE_LOG(LogAbyssDiverCharacter, Display, TEXT("RemoveRadialBlur"));
	
	AUnderwaterCharacter* OwnerCharacter = GetOwner<AUnderwaterCharacter>();
	UCameraComponent* ApplyCameraComponent = OwnerCharacter ? OwnerCharacter->GetFirstPersonCameraComponent() : nullptr;
	if (!RadialBlurMID || !ApplyCameraComponent)
	{
		return;
	}

	TArray<FWeightedBlendable>& PostProcessArr = ApplyCameraComponent->PostProcessSettings.WeightedBlendables.Array;
	PostProcessArr.RemoveAll([this](const FWeightedBlendable& Blendable)
	{
		return Blendable.Object == RadialBlurMID;
	});
	RadialBlurMID = nullptr;
}

void UCameraEffectComponent::UpdateRadialBlur()
{
	if (!RadialBlurMID)
	{
		GetWorld()->GetTimerManager().ClearTimer(RadialBlurTimerHandle);
		return;
	}

	RadialBlurAlpha = FMath::FInterpTo(RadialBlurAlpha, RadialBlurTargetAlpha, RadialBlurUpdateInterval, RadialBlurFadeSpeed);
	RadialBlurMID->SetScalarParameterValue(RadialBlurSpreadParam, RadialBlurAlpha * RadialBlurTargetValue);
	UE_LOG(LogAbyssDiverCharacter, Verbose, TEXT("Update RadialBlur : RadialBlurAlpha=%.2f"), RadialBlurAlpha);
	if (FMath::IsNearlyEqual(RadialBlurAlpha, RadialBlurTargetAlpha, 0.01f))
	{
		UE_LOG(LogAbyssDiverCharacter, Display, TEXT("Finish RadialBlur Fade : RadialBlurAlpha=%.2f"), RadialBlurAlpha);
		GetWorld()->GetTimerManager().ClearTimer(RadialBlurTimerHandle);
		if (FMath::IsNearlyZero(RadialBlurAlpha))
		{
			RemoveRadialBlur();
		}
	}
}

