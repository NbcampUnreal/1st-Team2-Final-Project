#include "MissionManagerComponent.h"
#include "MissionEventHubComponent.h"
#include "Framework/ADInGameState.h"
#include "Subsystems/MissionSubsystem.h"
#include "Missions/MissionBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UMissionManagerComponent::UMissionManagerComponent()
{
    SetIsReplicatedByDefault(true); // ������Ʈ ���� Ȱ��ȭ
}

void UMissionManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AADInGameState* GS = GetWorld() ? GetWorld()->GetGameState<AADInGameState>() : nullptr)
    {
        Hub = GS->FindComponentByClass<UMissionEventHubComponent>();
    }

    if (const UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            MissionSubsystem = GI->GetSubsystem<UMissionSubsystem>();
        }
    }

    const bool bServer = (GetOwner() && GetOwner()->HasAuthority());
    if (bServer)
    {
        WireHubEvents();
    }
}

void UMissionManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMissionManagerComponent, ActiveStates);
}

void UMissionManagerComponent::S_SubmitMissions_Implementation(const TArray<FMissionData>& Selected)
{
    UnbindAll();
    ActiveMissions.Reset();
    ActiveStates.Reset(Selected.Num());

    for (const FMissionData& Choice : Selected)
    {
        if (UMissionBase* M = CreateAndInitMission(Choice))
        {
            // �̼� �Ϸ� �� �Ŵ����� �ݹ�
            M->OnCompleteMissionDelegate.BindUObject(this, &UMissionManagerComponent::HandleMissionComplete);
            ActiveMissions.Add(M);

            FMissionRuntimeState S;
            S.MissionType = Choice.MissionType;
            S.MissionIndex = Choice.MissionIndex;
            // S.Goal = <DT���� �о� ä���>;  // MissionSubsystem�κ��� �о� ����
            ActiveStates.Add(S);
        }
    }

    BindAll();
    OnRep_Missions(); // ���������� ��� UI Ʈ����(����)
}

void UMissionManagerComponent::OnRep_Missions()
{
    // Ŭ��: HUD/Tablet ���ΰ�ħ (���ε��� ����/������Ʈ���� �о� ���)
}

void UMissionManagerComponent::SetProgress(uint8 Slot, int32 NewCurrent, int32 NewGoal, bool bForceRep)
{
    if (!ActiveStates.IsValidIndex(Slot)) return;
    ActiveStates[Slot].Current = NewCurrent;
    ActiveStates[Slot].Goal = NewGoal;
    if (bForceRep) { OnRep_Missions(); }
}

void UMissionManagerComponent::HandleMissionComplete(const EMissionType& Type, const uint8& Index)
{
    for (FMissionRuntimeState& S : ActiveStates)
    {
        if (S.MissionType == Type && S.MissionIndex == Index)
        {
            S.bCompleted = 1;
            S.Current = S.Goal;
            break;
        }
    }
    OnRep_Missions();
}

UMissionBase* UMissionManagerComponent::CreateAndInitMission(const FMissionData& Choice)
{
    const EMissionType Type = Choice.MissionType;
    const uint8 Index = Choice.MissionIndex;

    switch (Type)
    {
    case EMissionType::AggroTrigger:
    {
        // ����: NewObject<UAggroTriggerMission>(this) + DT���� AggroMissionParams ���� + Init
        // �ҽ� ��ġ: Subsystem�� switch ��� ����. 
        // return NewMission;
        break;
    }
    case EMissionType::KillMonster:
    {
        // Subsystem ���� �״�� �̽� (DT���� UnitId/Interval �� �о� Init) 
        break;
    }
    case EMissionType::Interaction:
    {
        // Subsystem ���� �״�� �̽� (InteractionMissionParams) 
        break;
    }
    case EMissionType::ItemCollection:
    {
        // Subsystem ���� �״�� �̽� (ItemCollectMissionParams) 
        break;
    }
    case EMissionType::ItemUse:
    {
        // Subsystem ���� �״�� �̽� (ItemUseMissionParams) 
        break;
    }
    default: break;
    }
    return nullptr;
}

// �Ʒ� 3�� �Լ��� ������Ʈ �̺�Ʈ�� �°� ����
void UMissionManagerComponent::BindAll() 
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        WireHubEvents();
    }
}


void UMissionManagerComponent::UnbindAll() {
    if (!Hub) return;
    Hub->OnItemCollected.RemoveAll(this);
    Hub->OnItemUsed.RemoveAll(this);
    Hub->OnInteracted.RemoveAll(this);
    Hub->OnMonsterKilled.RemoveAll(this);
    Hub->OnAggroTriggered.RemoveAll(this);
}
void UMissionManagerComponent::WireHubEvents()
{
    if (!(GetOwner() && GetOwner()->HasAuthority()) || !Hub) return; 
    // ��: ������ ���� �����
    Hub->OnItemCollected.AddLambda([this](uint8 ItemId, int32 Amount)
        {
            for (int32 i = 0; i < ActiveMissions.Num(); ++i)
            {
                if (UMissionBase* M = ActiveMissions[i])
                {
                    // �� �̼� �Ļ��� Notify �Լ� �߰��ؼ� ȣ��(��: UItemCollectionMission::NotifyItemCollected)
                }
            }
        });

    // OnItemUsed / OnInteracted / OnMonsterKilled / OnAggroTriggered �� ���� ����
}
