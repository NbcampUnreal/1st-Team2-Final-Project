#include "Boss/Effect/PostProcessSettingComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "Camera/CameraComponent.h"
#include "Character/UnderwaterCharacter.h"

UPostProcessSettingComponent::UPostProcessSettingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UPostProcessSettingComponent::BeginPlay()
{
	Super::BeginPlay();
    
    AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(GetOwner());
    if (!IsValid(Player))
    {
        LOG(TEXT("PostProcessSettingComponent must be attached to an UnderwaterCharacter!"));
        return;
    }
    
    CameraComponent = Player->GetFirstPersonCameraComponent();
    if (!IsValid(CameraComponent))
	{
		LOG(TEXT("PostProcessSettingComponent requires a CameraComponent on the owner actor!"));
	}

    CameraComponent->PostProcessSettings.bOverride_VignetteIntensity = true;
}

void UPostProcessSettingComponent::C_ActivateVignetteEffect_Implementation()
{
    ActivateVignetteEffect();
}

void UPostProcessSettingComponent::C_DeactivateVignetteEffect_Implementation()
{
    DeactivateVignetteEffect();
}

void UPostProcessSettingComponent::ActivateVignetteEffect()
{
    AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(GetOwner());
    if (!IsValid(Player) || !Player->IsLocallyControlled())
    {
        LOG(TEXT("ActivateVignetteEffect called on Server - this should not happen!"));
        return;
    }
    
    if (!IsValid(CameraComponent)) return;

    ++VignetteReferenceCount;
    
    if (VignetteReferenceCount > 1) return;
    
    // 현재 전환 중이면 타이머 정리
    if (bIsTransitioning)
    {
        GetWorld()->GetTimerManager().ClearTimer(VignetteTimerHandle);
    }
    
    // 현재 Vignette 강도 가져오기
    StartVignetteIntensity = CameraComponent->PostProcessSettings.VignetteIntensity;
    TargetVignetteIntensity = 1.0f;
    CurrentTransitionTime = 0.0f;
    bIsTransitioning = true;
    
    // 타이머 시작 (60fps로 업데이트)
    GetWorld()->GetTimerManager().SetTimer(
        VignetteTimerHandle,
        this,
        &UPostProcessSettingComponent::UpdateVignetteIntensity,
        1.0f / 60.0f, // 60fps
        true // 반복
    );
}

void UPostProcessSettingComponent::DeactivateVignetteEffect()
{
    AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(GetOwner());
    if (!IsValid(Player) || !Player->IsLocallyControlled())
    {
        LOG(TEXT("ActivateVignetteEffect called on Server - this should not happen!"));
        return;
    }
    
    if (!IsValid(CameraComponent)) return;

    --VignetteReferenceCount;

    if(VignetteReferenceCount > 0) return;

    // 현재 전환 중이면 타이머 정리
    if (bIsTransitioning)
    {
        GetWorld()->GetTimerManager().ClearTimer(VignetteTimerHandle);
    }
    
    // 현재 Vignette 강도 가져오기
    StartVignetteIntensity = CameraComponent->PostProcessSettings.VignetteIntensity;
    TargetVignetteIntensity = 0.4f;
    CurrentTransitionTime = 0.0f;
    bIsTransitioning = true;
    
    // 타이머 시작 (60fps로 업데이트)
    GetWorld()->GetTimerManager().SetTimer(
        VignetteTimerHandle,
        this,
        &UPostProcessSettingComponent::UpdateVignetteIntensity,
        1.0f / 60.0f, // 60fps
        true // 반복
    );
}

void UPostProcessSettingComponent::UpdateVignetteIntensity()
{
    if (!IsValid(CameraComponent)) return;

    CurrentTransitionTime += 1.0f / 60.0f; // 델타 타임 증가
    
    // 전환 완료 체크
    if (CurrentTransitionTime >= TransitionDuration)
    {
        // 최종 값으로 설정
        CameraComponent->PostProcessSettings.VignetteIntensity = TargetVignetteIntensity;
        
        // 타이머 정리
        GetWorld()->GetTimerManager().ClearTimer(VignetteTimerHandle);
        bIsTransitioning = false;
        return;
    }
    
    // 보간 계산 (부드러운 전환을 위해 SmoothStep 사용)
    float Alpha = CurrentTransitionTime / TransitionDuration;
    Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha); // 부드러운 곡선 보간
    
    // 현재 Vignette 강도 설정
    float CurrentIntensity = FMath::Lerp(StartVignetteIntensity, TargetVignetteIntensity, Alpha);
    CameraComponent->PostProcessSettings.VignetteIntensity = CurrentIntensity;

    LOG(TEXT("Vignette Intensity: %f"), CurrentIntensity);
}