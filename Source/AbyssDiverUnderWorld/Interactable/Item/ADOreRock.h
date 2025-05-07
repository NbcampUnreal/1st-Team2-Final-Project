#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "ADOreRock.generated.h"

class AADItemBase;

USTRUCT(BlueprintType)
struct FDropEntry : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnWeight = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AADItemBase> ItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MinCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinMass = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMass = 15.f;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADOreRock : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AADOreRock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region Method
public:
	virtual void Interact(AActor* InstigatorActor) override;
	
	void HandleMineRequest(APawn* InstigatorPawn);

	void SpawnDrops();

	void OnAssetLoaded(FDropEntry* Entry, float Mass);

protected:
	UFUNCTION()
	void OnRep_RemainingMines();

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion

#pragma region Variable
public:
	UPROPERTY(ReplicatedUsing=OnRep_RemainingMines, EditAnywhere, Category = "Mining")
	int32 RemainingMines = 3;
	UPROPERTY(EditDefaultsOnly, Category = "Drops")
	TObjectPtr<UDataTable> DropTable;
	UPROPERTY(EditDefaultsOnly, Category = "Mining")
	TObjectPtr<UParticleSystem> FractureFX;
	UPROPERTY(EditDefaultsOnly, Category = "Mining")
	TObjectPtr<USoundBase> FractureSound;

protected:


private:
	TArray<FDropEntry*> CachedEntries;
	TArray<float>  CumulativeWeights;
	float TotalWeight = 0.f;
	FStreamableManager Streamable;

#pragma endregion

#pragma region Getter, Setteer
public:

#pragma endregion
};
