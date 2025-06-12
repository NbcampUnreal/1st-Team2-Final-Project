#include "Missions/KillMonsterMission.h"

#include "DataRow/MissionDataRow/KillMonsterMissionRow.h"
#include "Character/UnderwaterCharacter.h"
#include "Monster/Monster.h"

UKillMonsterMission::UKillMonsterMission()
{
	MissionType = EMissionType::KillMonster;
}

void UKillMonsterMission::InitMission(const FMissionInitParams& Params)
{
	Super::InitMission(Params);
}

void UKillMonsterMission::InitMission(const FKillMissionInitParams& Params, const EKillMonsterMission& NewMissionIndex)
{
	InitMission((const FMissionInitParams&)Params);

	MissionIndex = NewMissionIndex;

	UnitId = Params.UnitId;
}

void UKillMonsterMission::BindDelegates(UObject* TargetForDelegate)
{
	if (TargetForDelegate->IsA<AMonster>())
	{
		AMonster* Monster = Cast<AMonster>(TargetForDelegate);
		Monster->OnMonsterDead.RemoveAll(this);
		Monster->OnMonsterDead.AddDynamic(this, &UKillMonsterMission::OnMonsterDead);
	}
}

void UKillMonsterMission::UnbindDelegates(UObject* TargetForDelegate)
{
	if (TargetForDelegate->IsA<AMonster>())
	{
		AMonster* Monster = Cast<AMonster>(TargetForDelegate);
		Monster->OnMonsterDead.RemoveAll(this);
	}
}

void UKillMonsterMission::OnMonsterDead(AActor* Killer, AMonster* DeadMonster)
{
	OnDead(Killer, (AActor*)DeadMonster);
}

bool UKillMonsterMission::IsConditionMet()
{
	bool bConditionMet = false;
	
	switch (ConditionType)
	{
	case EMissionConditionType::AtLeast:
		bConditionMet = (GoalCount <= CurrentCount);
		break;
	case EMissionConditionType::AtMost:
		bConditionMet = (GoalCount >= CurrentCount);
		break;
	case EMissionConditionType::EqualTo:
		bConditionMet = (GoalCount == CurrentCount);
		break;
	case EMissionConditionType::Custom:
		break;
	default:
		break;
	}

	return bConditionMet;
}

void UKillMonsterMission::OnDamaged(AActor* DamagedActor, AActor* DamageCauser, const float& ReceivedDamage)
{
}

void UKillMonsterMission::OnDead(AActor* DamageCauser, AActor* DeadActor)
{
	/*AUnderwaterCharacter* Causer = Cast<AUnderwaterCharacter>(DamageCauser);
	if (Causer == nullptr)
	{
		LOGV(Log, TEXT("Causer == nullptr, Name : %s"), *DamageCauser->GetName());
		return;
	}*/

	AUnitBase* DeadUnit = Cast<AUnitBase>(DeadActor);
	if (DeadUnit == nullptr)
	{
		LOGV(Log, TEXT("DeadUnit == nullptr, Name : %s"), *DeadActor->GetName());
		return;
	}

	const EUnitId DeadUnitId = DeadUnit->GetUnitId();

	if (DeadUnitId != UnitId)
	{
		return;
	}

	CurrentCount++;

	if (IsConditionMet() == false)
	{
		return;
	}

	OnConditionMet();
}

const uint8 UKillMonsterMission::GetMissionIndex() const
{
	return uint8(MissionIndex);
}
