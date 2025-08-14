// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNodes/AnimGraphNode_AquaticBoneTrail.h"
#include "AnimationGraphSchema.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Animation/AnimInstance.h"

#define LOCTEXT_NAMESPACE "AnimGraphNode_AquaticBoneTrail"

UAnimGraphNode_AquaticBoneTrail::UAnimGraphNode_AquaticBoneTrail(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

FText UAnimGraphNode_AquaticBoneTrail::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
    {
        return LOCTEXT("NodeTitle", "Aquatic Bone Trail");
    }
    else
    {
        FString BoneCount = FString::FromInt(Node.BoneNames.Num());
        FText Title = FText::Format(LOCTEXT("NodeTitleFormat", "Aquatic Bone Trail ({0} bones)"), FText::FromString(BoneCount));

        if (Node.BoneNames.Num() == 0)
        {
            Title = LOCTEXT("NodeTitleNoBones", "Aquatic Bone Trail (No bones)");
        }

        return Title;
    }
}

FText UAnimGraphNode_AquaticBoneTrail::GetTooltipText() const
{
    return LOCTEXT("NodeTooltip",
        "Applies trajectory-based offsets to multiple bones for creating natural swimming motion.\n"
        "Each bone follows the movement path with a time delay, creating a snake-like motion.");
}

FString UAnimGraphNode_AquaticBoneTrail::GetNodeCategory() const
{
    return TEXT("Aquatic Movement");
}

void UAnimGraphNode_AquaticBoneTrail::CreateOutputPins()
{
    const UAnimationGraphSchema* Schema = GetDefault<UAnimationGraphSchema>();
    CreatePin(EGPD_Output, Schema->PC_Struct, TEXT(""), FComponentSpacePoseLink::StaticStruct(), TEXT("Pose"));
}

FText UAnimGraphNode_AquaticBoneTrail::GetControllerDescription() const
{
    return LOCTEXT("ControllerDescription", "Aquatic Bone Trail");
}

void UAnimGraphNode_AquaticBoneTrail::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
    Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);

    // 본 이름 검증
    if (Node.BoneNames.Num() == 0)
    {
        MessageLog.Warning(*LOCTEXT("NoBones", "@@ has no bones specified").ToString(), this);
    }
    else
    {
        // 각 본이 스켈레톤에 존재하는지 확인
        for (const FName& BoneName : Node.BoneNames)
        {
            if (ForSkeleton && ForSkeleton->GetReferenceSkeleton().FindBoneIndex(BoneName) == INDEX_NONE)
            {
                FText Msg = FText::Format(LOCTEXT("NoSuchBone", "@@ - Bone '{0}' not found in Skeleton '{1}'"),
                    FText::FromName(BoneName),
                    FText::FromString(GetNameSafe(ForSkeleton)));
                MessageLog.Warning(*Msg.ToString(), this);
            }
        }
    }

    // Movement Component 태그 검증
    if (Node.MovementComponentTag.IsNone())
    {
        MessageLog.Warning(*LOCTEXT("NoComponentTag", "@@ has no Movement Component Tag specified. Using default search.").ToString(), this);
    }
}

void UAnimGraphNode_AquaticBoneTrail::CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const
{
    Super::CustomizePinData(Pin, SourcePropertyName, ArrayIndex);
}

void UAnimGraphNode_AquaticBoneTrail::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // 프로퍼티 변경 시 노드 타이틀 업데이트
    if (PropertyChangedEvent.Property != nullptr)
    {
        if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FAnimNode_AquaticBoneTrail, BoneNames))
        {
            ReconstructNode();
        }
    }
}

#undef LOCTEXT_NAMESPACE
