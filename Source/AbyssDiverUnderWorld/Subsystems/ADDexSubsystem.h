#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataRow/MonsterDexRow.h"
#include "ADDexSubsystem.generated.h"

class UDataTable;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADDexSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Dex")
	bool UnlockMonster(FName MonsterId);


	UFUNCTION(BlueprintCallable, Category = "Dex")
	bool IsUnlocked(FName MonsterId) const;

	const FMonsterDexRow* GetMonsterRow(FName MonsterId) const;

	void SaveDexData();

	void LoadDexData();

	void ResetDexData();

	void ApplyDexBitsSnapshot(const TArray<uint8>& InBits);

	const TArray<uint8>& GetDexBitsSnapshot() const { return DexBits; };

protected:

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> MonsterDexTable = nullptr;

	UPROPERTY()
	TSet <FName> UnlockedMonsterIds;

	UPROPERTY()
	TArray<uint8> DexBits; // 비트맵 형태로 몬스터 정보 저장

	UPROPERTY()
	TMap<FName, int32> IdToIndex;
};
