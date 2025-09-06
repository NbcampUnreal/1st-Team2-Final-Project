#include "MissionManagerComponent.h"
#include "MissionEventHubComponent.h"
#include "Framework/ADInGameState.h"
#include "Subsystems/MissionSubsystem.h"
#include "Missions/MissionBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UMissionManagerComponent::UMissionManagerComponent()
{
    SetIsReplicatedByDefault(true); // 컴포넌트 복제 활성화
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
            // 미션 완료 → 매니저로 콜백
            M->OnCompleteMissionDelegate.BindUObject(this, &UMissionManagerComponent::HandleMissionComplete);
            ActiveMissions.Add(M);

            FMissionRuntimeState S;
            S.MissionType = Choice.MissionType;
            S.MissionIndex = Choice.MissionIndex;
            // S.Goal = <DT에서 읽어 채우기>;  // MissionSubsystem로부터 읽어 세팅
            ActiveStates.Add(S);
        }
    }

    BindAll();
    OnRep_Missions(); // 서버에서도 즉시 UI 트리거(선택)
}

void UMissionManagerComponent::OnRep_Missions()
{
    // 클라: HUD/Tablet 새로고침 (바인딩한 위젯/컴포넌트에서 읽어 사용)
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
        // 기존: NewObject<UAggroTriggerMission>(this) + DT에서 AggroMissionParams 구성 + Init
        // 소스 위치: Subsystem의 switch 블록 참고. 
        // return NewMission;
        break;
    }
    case EMissionType::KillMonster:
    {
        // Subsystem 로직 그대로 이식 (DT에서 UnitId/Interval 등 읽어 Init) 
        break;
    }
    case EMissionType::Interaction:
    {
        // Subsystem 로직 그대로 이식 (InteractionMissionParams) 
        break;
    }
    case EMissionType::ItemCollection:
    {
        // Subsystem 로직 그대로 이식 (ItemCollectMissionParams) 
        break;
    }
    case EMissionType::ItemUse:
    {
        // Subsystem 로직 그대로 이식 (ItemUseMissionParams) 
        break;
    }
    default: break;
    }
    return nullptr;
}

// 아래 3개 함수는 프로젝트 이벤트에 맞게 구현
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
    // 예: 아이템 수집 라우팅
    Hub->OnItemCollected.AddLambda([this](uint8 ItemId, int32 Amount)
        {
            for (int32 i = 0; i < ActiveMissions.Num(); ++i)
            {
                if (UMissionBase* M = ActiveMissions[i])
                {
                    // 각 미션 파생에 Notify 함수 추가해서 호출(예: UItemCollectionMission::NotifyItemCollected)
                }
            }
        });

    // OnItemUsed / OnInteracted / OnMonsterKilled / OnAggroTriggered 도 동일 패턴
}
