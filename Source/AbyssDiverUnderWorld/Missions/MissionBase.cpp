#include "Missions/MissionBase.h"

#include "DataRow/MissionDataRow/MissionBaseRow.h"

void UMissionBase::InitMission(const FMissionInitParams& Params)
{
    MissionType = Params.MissionType;
    GoalCount = Params.GoalCount;
    CurrentCount = 0;

    ConditionType = Params.ConditionType;
    MissionName = Params.MissionName;
    MissionDescription = Params.MissionDescription;
    ExtraValues = Params.ExtraValues;

    bIsCompleted = 0; // (구) bIsCompletedAlready와 불일치 수정  :contentReference[oaicite:2]{index=2} :contentReference[oaicite:3]{index=3}

    if (Params.bCompleteInstantly)
    {
        CompleteMission();
    }
}

void UMissionBase::AddProgress(int32 Delta)
{
    if (bIsCompleted || GoalCount <= 0) return;

    const int32 Old = CurrentCount;
    CurrentCount = FMath::Clamp(CurrentCount + Delta, 0, GoalCount);

    if (CurrentCount != Old)
    {
        OnMissionProgressDelegate.ExecuteIfBound(MissionType, GetMissionIndex(), CurrentCount, GoalCount);
        OnMissionProgressChanged(CurrentCount - Old);
    }
    if (CurrentCount >= GoalCount)
    {
        CompleteMission();
    }
}

void UMissionBase::CompleteMission()
{
    if (bIsCompleted) return;      
    bIsCompleted = 1;

    OnMissionCompleted();    
    OnCompleteMissionDelegate.ExecuteIfBound(MissionType, GetMissionIndex());
}


void UMissionBase::OnMissionProgressChanged(int32 Delta)
{
  
}

void UMissionBase::OnMissionCompleted()
{
}

