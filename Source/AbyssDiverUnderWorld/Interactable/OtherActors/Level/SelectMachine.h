// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SelectMachine.generated.h"

enum class EMapName : uint8;
class UWidgetComponent;
struct FMissionData;
class AInteractableButton;
class USphereComponent;

UENUM(BlueprintType)
enum class ESelectMachineStateType : uint8
{
	CloseState,
	MapOpenState,
	MissionOpenState
};

USTRUCT(BlueprintType)
struct FMapInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMapName MapEnumType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsUnlocked : 1;
}; 
	
UCLASS()
class ABYSSDIVERUNDERWORLD_API ASelectMachine : public AActor
{
	GENERATED_BODY()
	
public:	
	ASelectMachine();

#pragma region Methods
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnOpenWindowOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOpenWindowOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintNativeEvent)
	void UpdateWidgetReaction(ESelectMachineStateType OldType, ESelectMachineStateType NewType);
	void UpdateWidgetReaction_Implementation(ESelectMachineStateType OldType, ESelectMachineStateType NewType);

	UFUNCTION(BlueprintNativeEvent)
	void UpdatelevelImage();
	void UpdatelevelImage_Implementation(); 

	void HandlePrevLevel(AActor* InteractInstigator);
	void HandleNextLevel(AActor* InteractInstigator);
	void AutoSelectLevel(AActor* InteractInstigator);
	void HandleTravelLevel(AActor* InteractInstigator);
	void SwitchbSelectMission(const TArray<FMissionData>& MissionsFromUI);
	void UpdateButtonDescription();

	bool IsConditionMet();
#pragma endregion

#pragma region Variables

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWidgetComponent> SelectMissionWidgetComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsEnterHost : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USphereComponent> OpenWindowCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataTable> MapInfoDataTable;

	int8 CurrentLevelIndex = 0;
private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AInteractableButton> GameStartButton;


	TArray<FMapInfo> LevelInfos;
	TArray<uint8> LevelUnLockInfos;

	uint8 bSelectLevel : 1 = false;
	uint8 bSelectMission : 1 = false;

	EMapName LevelID;

	ESelectMachineStateType SelectMachineStateType;
#pragma endregion

#pragma region Getter/Setter
public:
	UFUNCTION(BlueprintCallable)
	uint8 GetCurrentLevelIndex() { return CurrentLevelIndex; }
	void SetSelectMachineStateType(ESelectMachineStateType StateType);
	void SetbSelectMissionToTrue() { bSelectMission = true; };
#pragma endregion
};
