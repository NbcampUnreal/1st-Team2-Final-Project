#include "Subsystems/ADDexSubsystem.h"
#include "AbyssDiverUnderWorld.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/SaveDataSubsystem.h"
#include "Framework/ADGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"          
#include "DataRow/MonsterDexRow.h"      

void UADDexSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UADGameInstance* ADGI = Cast<UADGameInstance>(GetGameInstance());
	if (!ADGI)
	{
		LOG(TEXT("fail to cast into DexSubsystem UADGameInstance "));
		return;
	}
	MonsterDexTable = ADGI->GetMosterDexTable();
	if (!MonsterDexTable)
	{
		LOG( TEXT("[DexSubsystem] MonsterDexTable �ε� ���� - GameInstance���� ��ȯ�� �����Ͱ� nullptr"));
		return;
	}

	if (MonsterDexTable)
	{
		TArray<FMonsterDexRow*> Rows;
		MonsterDexTable->GetAllRows(TEXT(""), Rows);

		IdToIndex.Empty();
		for (int32 i = 0; i < Rows.Num(); ++i)
		{
			if (Rows[i])
			{
				IdToIndex.Add(Rows[i]->MonsterId, i);
			}
		}

		const int32 TotalMonsterCount = Rows.Num();
		DexBits.Init(0, FMath::DivideAndRoundUp(TotalMonsterCount, 8));
	}

	LoadDexData();
}

void UADDexSubsystem::Deinitialize()
{
	SaveDexData();
	Super::Deinitialize();
}

bool UADDexSubsystem::UnlockMonster(FName MonsterId)
{
	if (!UnlockedMonsterIds.Contains(MonsterId))
	{
		UnlockedMonsterIds.Add(MonsterId);

		if (const int32* Idx = IdToIndex.Find(MonsterId))
		{
			const int32 ByteIndex = *Idx / 8;
			const int32 BitIndex = *Idx % 8;
			if (DexBits.IsValidIndex(ByteIndex))
			{
				DexBits[ByteIndex] |= (1 << BitIndex);
			}
		}
		if (GetWorld() && GetWorld()->GetNetMode() != NM_Client)
		{
			SaveDexData();   // ����/ȣ��Ʈ/�̱ۿ����� ����
		}
		return true;
	}

	return false;
}

bool UADDexSubsystem::IsUnlocked(FName MonsterId) const
{
	return UnlockedMonsterIds.Contains(MonsterId);
}

const FMonsterDexRow* UADDexSubsystem::GetMonsterRow(FName MonsterId) const
{
	if (!MonsterDexTable)
	{
		LOG(TEXT("MonsterDexTable is not valid"));
		return nullptr;
	}

	const FMonsterDexRow* Row = MonsterDexTable->FindRow<FMonsterDexRow>(MonsterId, TEXT("DexLookup"));
	if (!Row)
	{
		LOG(TEXT("MontserID not Found in Data: %s"), *MonsterId.ToString());
		return nullptr;
	}
	return Row;
}

void UADDexSubsystem::SaveDexData()
{
	if (USaveDataSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveDataSubsystem>())
	{
		const int32 NeededBytes = FMath::DivideAndRoundUp(IdToIndex.Num(), 8);
		if (DexBits.Num() < NeededBytes)
		{
			const int32 OldNum = DexBits.Num();
			DexBits.SetNum(NeededBytes, false); // false�� �ʱ�ȭ�Ͽ� ���� �� ����
			for (int32 i = OldNum; i < NeededBytes; ++i)
			{
				DexBits[i] = 0;
			}
		}

		for (const TPair<FName, int32>& Pair : IdToIndex)
		{
			const int32 Index = Pair.Value;
			const int32 ByteIndex = Index / 8;
			const int32 BitIndex = Index % 8;

			if (UnlockedMonsterIds.Contains(Pair.Key))
			{
				DexBits[ByteIndex] |= (1 << BitIndex); // ��Ʈ ����
			}
			else
			{
				DexBits[ByteIndex] &= ~(1 << BitIndex); // ��Ʈ ����
			}
		}

		SaveSubsystem->SaveDexBits(DexBits);
		LOG(TEXT("[DexSubsystem] Saved Dex Data (%d bytes)"), DexBits.Num());
	}
}

void UADDexSubsystem::LoadDexData()
{
	if (USaveDataSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveDataSubsystem>())
	{
		if (SaveSubsystem->LoadDexBits(DexBits))
		{
			UnlockedMonsterIds.Empty();

			for (const TPair<FName, int32>& Pair : IdToIndex)
			{
				const int32 Index = Pair.Value;
				const int32 ByteIndex = Index / 8;
				const int32 BitIndex = Index % 8;

				if (DexBits.IsValidIndex(ByteIndex) && (DexBits[ByteIndex] & (1 << BitIndex)))
				{
					UnlockedMonsterIds.Add(Pair.Key);
				}
			}
		}
		else
		{
			LOG(TEXT("[DexSubsystem] Failed to Load Dex Data"));
		}
	}
	else
	{
		LOG(TEXT("[DexSubsystem] SaveSubsystem is not valid"));
	}
}

void UADDexSubsystem::ResetDexData()
{
	UnlockedMonsterIds.Empty();
	for (uint8& Byte : DexBits) Byte = 0;
	SaveDexData();
}

void UADDexSubsystem::ApplyDexBitsSnapshot(const TArray<uint8>& InBits)
{
	const int32 NeededBytes = FMath::DivideAndRoundUp(IdToIndex.Num(), 8);
	DexBits = InBits;
	if (DexBits.Num() < NeededBytes)
	{
		DexBits.SetNumZeroed(NeededBytes); // ������ �κ��� 0���� �ʱ�ȭ
	}
	else if (DexBits.Num() > NeededBytes)
	{
		DexBits.SetNum(NeededBytes, true); // �ʰ��� �κ��� �߶�
	}

	UnlockedMonsterIds.Empty();
	for (const TPair<FName, int32>&Pair: IdToIndex)
	{
		const int32 Index = Pair.Value;
		const int32 ByteIndex = Index / 8;
		const int32 BitIndex = Index % 8;

		if (DexBits.IsValidIndex(ByteIndex) && (DexBits[ByteIndex] & (1 << BitIndex)))
		{
			UnlockedMonsterIds.Add(Pair.Key);
		}
	}


}
