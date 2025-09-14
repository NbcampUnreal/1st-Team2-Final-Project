#include "Missions/KillMonsterMission.h"
#include "Engine/World.h"
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

	TargetUnitId = Params.UnitId;
	NeededSimultaneousKillCount = Params.NeededSimultaneousKillCount;
	KillInterval = Params.KillInterval;

	static const int32 PlayerCount = 4;
}

void UKillMonsterMission::NotifyMonsterKilled(EUnitId UnitId)
{
	if (IsCompleted())
		return;
	if (UnitId != TargetUnitId)
		return;

	// 단일 킬
	if (NeededSimultaneousKillCount <= 1 || KillInterval <= 0.f)
	{
		AddProgress(1);
	}

	// 동시 킬
	const float Now = GetWorld()->GetTimeSeconds();
	KillTimes.Add(Now);

	// 시간 창 밖 기록 제거
	while (KillTimes.Num() && (Now - KillTimes[0]) > KillInterval)
	{
		KillTimes.RemoveAt(0);
	}

	if (KillTimes.Num() >= NeededSimultaneousKillCount)
	{
		KillTimes.Reset(); // 한 번 충족 후 초기화(디자인에 맞게 조정)
		AddProgress(1);
	}

}


uint8 UKillMonsterMission::GetMissionIndex() const
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
