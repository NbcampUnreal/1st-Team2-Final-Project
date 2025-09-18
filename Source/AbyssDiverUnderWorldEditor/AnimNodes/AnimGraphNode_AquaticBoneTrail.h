// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "AnimNodes/AnimNode_AquaticBoneTrail.h"
#include "AnimGraphNode_AquaticBoneTrail.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLDEDITOR_API UAnimGraphNode_AquaticBoneTrail : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = Settings)
    FAnimNode_AquaticBoneTrail Node;        

public:
    // UEdGraphNode interface
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FText GetTooltipText() const override;
    // End of UEdGraphNode interface

    // UAnimGraphNode_Base interface
    virtual FString GetNodeCategory() const override;
    virtual void CreateOutputPins() override;
    // End of UAnimGraphNode_Base interface

    // UAnimGraphNode_SkeletalControlBase interface
    virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }
    // End of UAnimGraphNode_SkeletalControlBase interface

protected:
    // UAnimGraphNode_SkeletalControlBase interface
    virtual FText GetControllerDescription() const override;
    // End of UAnimGraphNode_SkeletalControlBase interface

    // UAnimGraphNode_Base interface
    virtual void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog) override;
    virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const override;
    // End of UAnimGraphNode_Base interface

    // UObject interface
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    // End of UObject interface

public:
    // Constructor
    UAnimGraphNode_AquaticBoneTrail(const FObjectInitializer& ObjectInitializer);
};