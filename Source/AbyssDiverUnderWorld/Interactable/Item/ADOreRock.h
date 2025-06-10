#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "ADOreRock.generated.h"

class AADItemBase;
class UNiagaraSystem;
class UNiagaraComponent;
class UADInteractableComponent;
class URadarReturnComponent;
class AUnderwaterCharacter;
class USoundSubsystem;

USTRUCT(BlueprintType)
struct FDropEntry : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Id = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnWeight = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AADItemBase> ItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MinCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MaxCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinMass = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxMass = 15;
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
	UFUNCTION(NetMulticast, Reliable)
	void M_CleanupToolAndEffects(AUnderwaterCharacter* UnderwaterCharacter);
	void M_CleanupToolAndEffects_Implementation(AUnderwaterCharacter* UnderwaterCharacter);

	UFUNCTION(NetMulticast, Reliable)
	void M_SpawnAndAttachTool(AUnderwaterCharacter* UnderwaterCharacter);
	void M_SpawnAndAttachTool_Implementation(AUnderwaterCharacter* UnderwaterCharacter);

	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual void InteractHold_Implementation(AActor* InstigatorActor) override;
	virtual void OnHoldStart_Implementation(APawn* InstigatorPawn) override;
	virtual void OnHoldStop_Implementation(APawn* InstigatorPawn) override;

	void HandleMineRequest(APawn* InstigatorPawn);

	void SpawnDrops();

	void OnAssetLoaded(FDropEntry* Entry, int32 Mass);

	void PlayMiningFX();
	void PlayFractureFX();

	// 편향된 무게 한 점을 반환하는 함수
	int32 SampleDropMass(int32 MinMass, int32 MaxMass) const;
	
	

protected:
	UFUNCTION()
	void OnRep_CurrentMiningGauge();

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PlayMiningAnim(APawn* InstigatorPawn);
	void PlayStowAnim(APawn* InstigatorPawn);
	

#pragma endregion

#pragma region Variable
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	uint8 bIsHold : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float HoldDuration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	int32 MaxMiningGauge = 100;           // 최대 게이지
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMiningGauge, BlueprintReadOnly, Category = "Mining")
	uint8 CurrentMiningGauge = 100;       // 현재 게이지
	UPROPERTY(EditDefaultsOnly, Category = "Drops")
	TObjectPtr<UDataTable> DropTable;
	UPROPERTY(EditDefaultsOnly, Category = "Mining")
	TObjectPtr<USoundBase> FractureSound;
	//도끼 타격 시 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Mining")
	TObjectPtr<UNiagaraSystem> PickAxeImpactFX;
	// 암석 파편 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Mining")
	TObjectPtr<UNiagaraSystem> RockFragmentsFX;
	UPROPERTY()
	TObjectPtr<USoundSubsystem> SoundSubsystem;

protected:
	// 무게 샘플링 강도 (1.0 : 균등, 2.0 : 중간 편향, 클수록 편향이 강해짐)
	UPROPERTY(EditAnywhere, Category = "Drops|Sampling")
	float MassBiasExponent = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Drops")
	float SpawnHeight = 150.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UADInteractableComponent> InteractableComp;

	UPROPERTY(EditAnywhere, Category = "Radar Settings")
	TObjectPtr<URadarReturnComponent> RadarReturnComponent;

	UPROPERTY(EditAnywhere, Category = "Mining")
	int32 DefaultMiningStrength = 25;

	// Mining Effect 관련 변수
	UPROPERTY(EditAnywhere, Category = "Mining")
	TSubclassOf<AActor>  MiningToolClass;
	UPROPERTY(EditAnywhere, Category = "Mining")
	TObjectPtr<UAnimMontage> MiningMontage;
	UPROPERTY(EditAnywhere, Category = "Mining")
	TObjectPtr<UAnimMontage> StowMontage;
	
private:
	TArray<FDropEntry*> CachedEntries;
	TArray<float>  CumulativeWeights;
	float TotalWeight = 0.f;
	int32 PreviousEquipIndex = 0;


#pragma endregion

#pragma region Getter, Setteer
public:
	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const;
	virtual float GetHoldDuration_Implementation() const override;
	virtual FString GetInteractionDescription() const override;

private:
	USoundSubsystem* GetSoundSubsystem();

#pragma endregion
};
