#include "Missions/KillMonsterMission.h"

#include "DataRow/MissionDataRow/KillMonsterMissionRow.h"
#include "Character/UnderwaterCharacter.h"
#include "Monster/Monster.h"
#include "Framework/ADPlayerState.h"
#include "Framework/ADPlayerController.h"

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
	NeededSimultaneousKillCount = Params.NeededSimultaneousKillCount;
	KillInterval = Params.KillInterval;

	static const int32 PlayerCount = 4;
	for (int32 i = 0; i < PlayerCount; ++i)
	{
		PlayerKillInfoArray.Add({ 0, 0 });
	}
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

	if (NeededSimultaneousKillCount == 1)
	{
		CurrentCount++;
	}
	else if (NeededSimultaneousKillCount > 1)
	{
		int32 PlayerIndex = GetPlayerIndex(DamageCauser);
		if (PlayerIndex == INDEX_NONE)
		{
			return;
		}

		float CurrentKillTime = FPlatformTime::Seconds();

		check(PlayerKillInfoArray.Num() > PlayerIndex);
		float& LastKilledTime = PlayerKillInfoArray[PlayerIndex].Key;
		uint8& CachedSimultaneousKillCount = PlayerKillInfoArray[PlayerIndex].Value;

		if (CurrentKillTime - LastKilledTime <= KillInterval)
		{
			CachedSimultaneousKillCount++;

			if (CachedSimultaneousKillCount >= NeededSimultaneousKillCount)
			{
				int32 IncreaseCount = CachedSimultaneousKillCount / NeededSimultaneousKillCount;
				CachedSimultaneousKillCount %= NeededSimultaneousKillCount;
				CurrentCount += IncreaseCount;
			}
		}
		else
		{
			CachedSimultaneousKillCount = 1;
		}
		LOGV(Warning, TEXT("Current: %f, Last : %f, Interval : %f"), CurrentKillTime, LastKilledTime, CurrentKillTime - LastKilledTime);
		LastKilledTime = CurrentKillTime;
	}
	else
	{
		ensureMsgf(false, TEXT("동시 사냥 킬카운트 잘못 입력되어있습니다. MissionType : %d, MissionIndex : %d, NeededSimultaneousKillCount : %d"), MissionType, MissionIndex, NeededSimultaneousKillCount);
		LOGV(Error, TEXT("동시 사냥 킬카운트 잘못 입력되어있습니다. MissionType : %d, MissionIndex : %d, NeededSimultaneousKillCount : %d"), MissionType, MissionIndex, NeededSimultaneousKillCount);
		return;
	}

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

int32 UKillMonsterMission::GetPlayerIndex(AActor* DamageCauser)
{
	if (DamageCauser == nullptr)
	{
		return INDEX_NONE;
	}

	int32 PlayerIndex = INDEX_NONE;

	AActor* SuspectedPlayer = DamageCauser;
	
	static const int32 TryCount = 5;
	for (int32 i = 0; i < TryCount; ++i)
	{
		PlayerIndex = GetPlayerIndexInternal(SuspectedPlayer);
		if (PlayerIndex != INDEX_NONE)
		{
			return PlayerIndex;
		}

		SuspectedPlayer = SuspectedPlayer->GetOwner();
		if (SuspectedPlayer == nullptr)
		{
			break;
		}
	}

	return INDEX_NONE;
}

int32 UKillMonsterMission::GetPlayerIndexInternal(AActor* SuspectedPlayer)
{
	if (SuspectedPlayer == nullptr)
	{
		LOGV(Log, TEXT("PlayerCharacter == nullptr"));
		return INDEX_NONE;
	}

	if (AADPlayerController* PlayerController = Cast<AADPlayerController>(SuspectedPlayer))
	{
		AADPlayerState* PS = PlayerController->GetPlayerState<AADPlayerState>();
		if (PS == nullptr)
		{
			LOGV(Error, TEXT("PS == nullptr"));
			return INDEX_NONE;
		}

		return PS->GetPlayerIndex();
	}

	AUnderwaterCharacter* PlayerCharacter = Cast<AUnderwaterCharacter>(SuspectedPlayer);
	if (PlayerCharacter == nullptr)
	{
		LOGV(Log, TEXT("PlayerCharacter == nullptr, Name : %s"), *SuspectedPlayer->GetName());
		return INDEX_NONE;
	}

	AADPlayerState* PS = Cast<AADPlayerState>(PlayerCharacter->GetPlayerState());
	if (PS == nullptr)
	{
		LOGV(Log, TEXT("PS == nullptr"));
		return INDEX_NONE;
	}

	return PS->GetPlayerIndex();
}
