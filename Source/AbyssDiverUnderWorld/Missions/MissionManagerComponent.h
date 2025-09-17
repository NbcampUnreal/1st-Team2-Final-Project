#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/MissionData.h"      
#include "GameplayTagContainer.h"
#include "GameplayTags/EquipNativeTags.h"
#include "MissionManagerComponent.generated.h"

class UMissionBase;
class UMissionEventHubComponent;
class UMissionSubsystem; // DT/īŻ�α� ��ȸ�� (���� ����ý���)
enum class EUnitId : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStatesUpdated, const TArray<FMissionRuntimeState>&, States);

USTRUCT(BlueprintType)
struct FMissionRuntimeState
{
	GENERATED_BODY()

	UPROPERTY() EMissionType MissionType = (EMissionType)0;
	UPROPERTY() uint8  MissionIndex = 0;
	UPROPERTY() int32  Current = 0;
	UPROPERTY() int32  Goal = 0;
	UPROPERTY() uint8  bCompleted : 1 = 0;
};

UCLASS( ClassGroup=(Mission), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UMissionManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMissionManagerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:
    UFUNCTION(BlueprintAuthorityOnly)
    void ApplySelectedMissions(const TArray<FMissionData>& Selected);
	

    // �� Ŭ�� HUD�� ������ ���� ���� ����
    UPROPERTY(ReplicatedUsing = OnRep_Missions)
    TArray<FMissionRuntimeState> ActiveStates;

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
    void BP_OnMissionStatesUpdated(const TArray<FMissionRuntimeState>& States);

    UFUNCTION()
    void OnRep_Missions(); // HUD/Tablet ���� Ʈ����

    // Hub -> Manager �ݹ�
    void HandleMonsterKilled(const FGameplayTagContainer& UnitTags);
    void HandleItemCollected(const FGameplayTagContainer& ItemTags, int32 Amount);
    void HandleItemUsed(const FGameplayTagContainer& ItemTags, int32 Amount);
    void HandleAggro(const FGameplayTagContainer& SourceTags);
    void HandleInteracted(const FGameplayTagContainer& InteractTags);

    UFUNCTION()
    void HandleMissionProgress(EMissionType MissionType, uint8 MissionIndex, int32 Current, int32 Goal);


    UPROPERTY(BlueprintAssignable)
    FOnMissionStatesUpdated OnMissionStatesUpdated;

    // Delegate Handles
    FDelegateHandle Handle_ItemCollected;
    FDelegateHandle Handle_ItemUsed;
    FDelegateHandle Handle_Interacted;
    FDelegateHandle Handle_MonsterKilled;
    FDelegateHandle Handle_Aggro;

private:
    // �������� �����ϴ� ���� �̼� ��ü��
    UPROPERTY(Transient)
    TArray<TObjectPtr<UMissionBase>> ActiveMissions;

    // ���(�̺�Ʈ ������, GameState�� ����)
    UPROPERTY()
    TObjectPtr<UMissionEventHubComponent> Hub;

    // ���������̺�/īŻ�α� ��ȸ�� (���� ����ý���)
    UPROPERTY()
    TObjectPtr<UMissionSubsystem> MissionSubsystem;

    // ���� Subsystem�� switch-����-Init ������ �̽��� �ڸ�
    UMissionBase* CreateAndInitMission(const FMissionData& Choice);

    // ��� ����/���� + �̺�Ʈ �����
    void BindAll();
    void UnbindAll();
    void WireHubEvents();

    // �Ϸ� �ݹ�(UMissionBase�� �̱�ĳ��Ʈ ��������Ʈ�� ȣ��)
    void HandleMissionComplete(EMissionType Type, uint8 Index);

    // ���ప ���� ����
    void SetProgress(uint8 Slot, int32 NewCurrent, int32 NewGoal, bool bForceRep = false);


private:
    uint8 bEventsBound : 1 = 0;

    void OnActiveMissionCountChanged();
};
