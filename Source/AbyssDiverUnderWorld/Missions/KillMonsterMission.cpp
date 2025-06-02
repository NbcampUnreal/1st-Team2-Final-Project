#include "Missions/KillMonsterMission.h"

#include "DataRow/MissionDataRow/KillMonsterMissionRow.h"

UKillMonsterMission::UKillMonsterMission()
{
	MissionType = EMissionType::KillMonster;
}

void UKillMonsterMission::InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex)
{
	InitMission(Params, (EKillMonsterMission)NewMissionIndex);
}

void UKillMonsterMission::InitMission(const FMissionInitParams& Params, const EKillMonsterMission& NewMissionIndex)
{
	MissionType = Params.MissionType;
	GoalCount = Params.GoalCount;
	ConditionType = Params.ConditionType;

	MissionName = Params.MissionName;
	MissionDescription = Params.MissionDescription;

	ExtraValues = Params.ExtraValues;

	MissionIndex = NewMissionIndex;
}

void UKillMonsterMission::BindDelegates(UObject* TargetForDelegate)
{
}

void UKillMonsterMission::UnbindDelegates(UObject* TargetForDelegate)
{
}

bool UKillMonsterMission::IsConditionMet()
{
	return false;
}

void UKillMonsterMission::OnConditionMet()
{
}

void UKillMonsterMission::OnDamaged(AActor* DamagedActor, AActor* DamageCauser, const float& ReceivedDamage)
{
}

void UKillMonsterMission::OnDead(AActor* DamageCauser, AActor* DeadActor)
{
}

const uint8 UKillMonsterMission::GetMissionIndex() const
{
	return uint8(MissionIndex);
}
