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
    // 추적할 본 이름 배열
    UPROPERTY(EditAnywhere, Category = "Trail")
    TArray<FName> BoneNames;

    // Movement Component 참조를 위한 태그 이름
    UPROPERTY(EditAnywhere, Category = "Trail")
    FName MovementComponentTag = "AquaticMovement";

    // 전역 스케일 팩터
    UPROPERTY(EditAnywhere, Category = "Trail", meta = (ClampMin = "0.0", ClampMax = "4.0"))
    float GlobalOffsetScale = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Trail", meta = (ClampMin = "0.0", ClampMax = "4.0"))
    float GlobalRotationScale = 1.0f;

    // 디버그 옵션
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bEnableDebugDraw = false;

protected:
    // 본 참조 캐시
    TArray<FBoneReference> BoneReferences;

    // 원래 인덱스에서 정렬된 인덱스로의 매핑
    TArray<int32> CachedBoneIndexMap;

    // 캐시된 컴포넌트 참조
    class UAquaticMovementComponent* CachedMovementComponent;

    // 마지막 업데이트 시간
    float LastUpdateTime;

    // 본 오프셋 캐시
    TArray<FVector> CachedOffsets;
    TArray<FRotator> CachedRotations;

private:
    // 헬퍼 함수들
    void FindMovementComponent(const FAnimationUpdateContext& Context);
    void ApplyBoneOffset(FComponentSpacePoseContext& Output, int32 BoneIndex, const FVector& Offset, const FRotator& Rotation, TArray<FBoneTransform>& OutBoneTransforms);
};