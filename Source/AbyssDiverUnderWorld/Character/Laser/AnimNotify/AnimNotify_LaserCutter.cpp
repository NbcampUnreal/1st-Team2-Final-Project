// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Laser/AnimNotify/AnimNotify_LaserCutter.h"
#include "AnimNotify_LaserCutter.h"
#include "Character/UnderwaterCharacter.h"

void UAnimNotify_LaserCutter::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    UE_LOG(LogTemp, Log, TEXT("MeshComp?"));
    if (!MeshComp) return;
    UE_LOG(LogTemp, Log, TEXT("Is MeshComp Owner : %s"), *MeshComp->GetOwner()->GetName());

    if (AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(MeshComp->GetOwner()))
    {
        Diver->CleanupToolAndEffects();
        UE_LOG(LogTemp, Log, TEXT("LaserCutter Notify ¡æ Cleanup OK"));
    }
}
