#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"  // FVector_NetQuantize, FQuat_NetQuantize를 위한 헤더 추가
#include "RagdollTypes.generated.h"

USTRUCT(BlueprintType)
struct FReplicatedBoneTransform
{
    GENERATED_BODY()

    UPROPERTY()
    FVector_NetQuantize Position;

    UPROPERTY()
    FRotator Rotation;
    
    UPROPERTY()
    FVector_NetQuantize LinearVelocity;
    
    UPROPERTY()
    FVector_NetQuantize AngularVelocity;

    FReplicatedBoneTransform(const FVector& InPosition = FVector::ZeroVector,
                            const FRotator& InRotation = FRotator::ZeroRotator,
                            const FVector& InLinearVelocity = FVector::ZeroVector,
                            const FVector& InAngularVelocity = FVector::ZeroVector)
        : Position(InPosition), Rotation(InRotation), LinearVelocity(InLinearVelocity), AngularVelocity(InAngularVelocity)
    {
    }

    FTransform ToTransform() const
    {
        return FTransform(Rotation, Position);
    }
};
