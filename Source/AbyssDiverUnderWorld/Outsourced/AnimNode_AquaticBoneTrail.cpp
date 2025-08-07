// Fill out your copyright notice in the Description page of Project Settings.


#include "Outsourced/AnimNode_AquaticBoneTrail.h"
#include "Animation/AnimInstanceProxy.h"
#include "Components/SkeletalMeshComponent.h"
#include "AquaticMovementComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

FAnimNode_AquaticBoneTrail::FAnimNode_AquaticBoneTrail()
    : GlobalOffsetScale(1.0f)
    , GlobalRotationScale(1.0f)
    , bEnableDebugDraw(false)
    , CachedMovementComponent(nullptr)
    , LastUpdateTime(0.0f)
{
}

void FAnimNode_AquaticBoneTrail::GatherDebugData(FNodeDebugData& DebugData)
{
    FString DebugLine = DebugData.GetNodeName(this);
    DebugLine += FString::Printf(TEXT(" - %d bones tracked"), BoneNames.Num());
    DebugData.AddDebugItem(DebugLine);

    ComponentPose.GatherDebugData(DebugData);
}


void FAnimNode_AquaticBoneTrail::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
    //SCOPE_CYCLE_COUNTER(STAT_AquaticBoneTrail_Eval);

    if (!CachedMovementComponent)
    {
        const FAnimInstanceProxy* Proxy = Output.AnimInstanceProxy;
        if (Proxy && Proxy->GetSkelMeshComponent())
        {
            AActor* Owner = Proxy->GetSkelMeshComponent()->GetOwner();
            if (Owner)
            {
                CachedMovementComponent = Owner->FindComponentByClass<UAquaticMovementComponent>();
            }
        }
    }

    if (!CachedMovementComponent)
    {
        return;
    }

    TArray<FVector> PositionDeltas;
    TArray<FRotator> RotationDeltas;
    CachedMovementComponent->GetBoneDeltas(PositionDeltas, RotationDeltas);

    const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

    TArray<FBoneTransform> TempTransforms;

    for (int32 i = 0; i < BoneNames.Num() && i < PositionDeltas.Num(); i++)
    {
        // 본 인덱스 찾기
        FName BoneName = BoneNames[i];
        const FReferenceSkeleton& RefSkeleton = Output.AnimInstanceProxy->GetSkeleton()->GetReferenceSkeleton();
        int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);

        if (BoneIndex == INDEX_NONE) continue;

        FCompactPoseBoneIndex CompactIndex = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneIndex));
        if (CompactIndex == INDEX_NONE) continue;

        // 현재 본 트랜스폼
        FTransform CurrentTransform = Output.Pose.GetComponentSpaceTransform(CompactIndex);

        // Translation 적용 (델타 추가)
        FVector NewPosition = CurrentTransform.GetLocation() + PositionDeltas[i] * GlobalOffsetScale;
        CurrentTransform.SetLocation(NewPosition);

        // Rotation 적용 - 월드 공간 델타를 컴포넌트 공간으로 변환 후 적용
        if (!RotationDeltas[i].IsNearlyZero())
        {
            // 월드 공간 회전 델타를 컴포넌트 공간으로 변환
            FTransform ComponentToWorld = Output.AnimInstanceProxy->GetComponentTransform();
            FQuat WorldDeltaQuat = (RotationDeltas[i] * GlobalRotationScale).Quaternion();
            
            // 월드 공간 델타를 컴포넌트 공간 델타로 변환
            FQuat ComponentDeltaQuat = ComponentToWorld.GetRotation().Inverse() * WorldDeltaQuat * ComponentToWorld.GetRotation();
            
            // 부모 본의 트랜스폼 가져오기
            int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);
            FTransform ParentTransform = FTransform::Identity;
            
            if (ParentBoneIndex != INDEX_NONE)
            {
                FCompactPoseBoneIndex ParentCompactIndex = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(ParentBoneIndex));
                if (ParentCompactIndex != INDEX_NONE)
                {
                    ParentTransform = Output.Pose.GetComponentSpaceTransform(ParentCompactIndex);
                }
            }
            
            // 로컬 공간으로 변환
            FTransform LocalTransform = CurrentTransform.GetRelativeTransform(ParentTransform);
            
            // 컴포넌트 공간 델타를 로컬 공간 델타로 변환
            FQuat LocalDeltaQuat = ParentTransform.GetRotation().Inverse() * ComponentDeltaQuat * ParentTransform.GetRotation();
            
            // 로컬 공간에서 회전 적용
            FQuat NewLocalRotation = LocalTransform.GetRotation() * LocalDeltaQuat;
            LocalTransform.SetRotation(NewLocalRotation);
            
            // 다시 컴포넌트 공간으로 변환
            CurrentTransform = LocalTransform * ParentTransform;
        }

        // 임시 저장
        TempTransforms.Add(FBoneTransform(CompactIndex, CurrentTransform));
    }

    TempTransforms.Sort([](const FBoneTransform& A, const FBoneTransform& B)
        {
            return A.BoneIndex.GetInt() < B.BoneIndex.GetInt();
        });

    OutBoneTransforms.Append(TempTransforms);
}

bool FAnimNode_AquaticBoneTrail::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{

    for (const FBoneReference& BoneRef : BoneReferences)
    {
        if (!BoneRef.IsValidToEvaluate(RequiredBones))
        {
            return false;
        }
    }

    return BoneNames.Num() > 0;
}

void FAnimNode_AquaticBoneTrail::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
    
    for (const FName& BoneName : BoneNames)
    {
        const FReferenceSkeleton& RefSkeleton = RequiredBones.GetReferenceSkeleton();
        int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);

        if (BoneIndex == INDEX_NONE)
        {
            UE_LOG(LogTemp, Warning, TEXT("Bone %s not found in skeleton"), *BoneName.ToString());
        }
    }
}