#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/MissionData.h"      
#include "MissionManagerComponent.generated.h"

class UMissionBase;
class UMissionEventHubComponent;
class UMissionSubsystem; // DT/īŻ�α� ��ȸ�� (���� ����ý���)

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
    // �� Ŭ��漭��: �º����� ���õ� �̼� ����
    UFUNCTION(Server, Reliable)
    void S_SubmitMissions(const TArray<FMissionData>& Selected);
	void S_SubmitMissions_Implementation(const TArray<FMissionData>& Selected);

    // �� Ŭ�� HUD�� ������ ���� ���� ����
    UPROPERTY(ReplicatedUsing = OnRep_Missions)
    TArray<FMissionRuntimeState> ActiveStates;

    UFUNCTION()
    void OnRep_Missions(); // HUD/Tablet ���� Ʈ����
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
    void HandleMissionComplete(const EMissionType& Type, const uint8& Index);

    // ���ప ���� ����
    void SetProgress(uint8 Slot, int32 NewCurrent, int32 NewGoal, bool bForceRep = false);


		
};
