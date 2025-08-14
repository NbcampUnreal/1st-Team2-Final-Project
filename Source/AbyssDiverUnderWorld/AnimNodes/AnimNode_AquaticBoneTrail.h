// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_AquaticBoneTrail.generated.h"

/**
 * 
 */
USTRUCT(BlueprintInternalUseOnly)
struct ABYSSDIVERUNDERWORLD_API FAnimNode_AquaticBoneTrail : public FAnimNode_SkeletalControlBase
{
    GENERATED_BODY()

public:
    FAnimNode_AquaticBoneTrail();

    // FAnimNode_Base interface
    virtual void GatherDebugData(FNodeDebugData& DebugData) override;
    // End of FAnimNode_Base interface

    // FAnimNode_SkeletalControlBase interface
    virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
    virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
    // End of FAnimNode_SkeletalControlBase interface

private:
    // FAnimNode_SkeletalControlBase interface
    virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
    // End of FAnimNode_SkeletalControlBase interface

public:
    // ������ �� �̸� �迭
    UPROPERTY(EditAnywhere, Category = "Trail")
    TArray<FName> BoneNames;

    // Movement Component ������ ���� �±� �̸�
    UPROPERTY(EditAnywhere, Category = "Trail")
    FName MovementComponentTag = "AquaticMovement";

    // ���� ������ ����
    UPROPERTY(EditAnywhere, Category = "Trail", meta = (ClampMin = "0.0", ClampMax = "4.0"))
    float GlobalOffsetScale = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Trail", meta = (ClampMin = "0.0", ClampMax = "4.0"))
    float GlobalRotationScale = 1.0f;

    // ����� �ɼ�
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bEnableDebugDraw = false;

protected:
    // �� ���� ĳ��
    TArray<FBoneReference> BoneReferences;

    // ���� �ε������� ���ĵ� �ε������� ����
    TArray<int32> CachedBoneIndexMap;

    // ĳ�õ� ������Ʈ ����
    class UAquaticMovementComponent* CachedMovementComponent;

    // ������ ������Ʈ �ð�
    float LastUpdateTime;

    // �� ������ ĳ��
    TArray<FVector> CachedOffsets;
    TArray<FRotator> CachedRotations;

private:
    // ���� �Լ���
    void FindMovementComponent(const FAnimationUpdateContext& Context);
    void ApplyBoneOffset(FComponentSpacePoseContext& Output, int32 BoneIndex, const FVector& Offset, const FRotator& Rotation, TArray<FBoneTransform>& OutBoneTransforms);
};