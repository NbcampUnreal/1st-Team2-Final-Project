#include "MissionManagerComponent.h"
#include "MissionEventHubComponent.h"
#include "Framework/ADInGameState.h"
#include "Subsystems/MissionSubsystem.h"
#include "Missions/MissionBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Character/UnitBase.h"

//Mission includes
#include "MissionBase.h"
#include "AggroTriggerMission.h"
#include "InteractionMission.h"
#include "ItemCollectionMission.h"
#include "ItemUseMission.h"
#include "KillMonsterMission.h"

#define AUTH_GUARD if (!(GetOwner() && GetOwner()->HasAuthority())) return;

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

}

void UMissionManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMissionManagerComponent, ActiveStates);
}

void UMissionManagerComponent::ApplySelectedMissions(const TArray<FMissionData>& Selected)
{
    UnbindAll();
    ActiveMissions.Reset();
    ActiveStates.Reset(Selected.Num());

    for (const FMissionData& Choice : Selected)
    {
        if (UMissionBase* Mission = CreateAndInitMission(Choice))
        {
            // 미션 완료 → 매니저로 콜백
            Mission->OnCompleteMissionDelegate.BindUObject(this, &UMissionManagerComponent::HandleMissionComplete);
            Mission->OnMissionProgressDelegate.BindUObject(this, &UMissionManagerComponent::HandleMissionProgress); // ★ 추가
            ActiveMissions.Add(Mission);

            FMissionRuntimeState State;
            State.MissionType = Choice.MissionType;
            State.MissionIndex = Choice.MissionIndex;
            State.Current = 0;
            State.bCompleted = false;
            // S.Goal = <DT에서 읽어 채우기>;  // MissionSubsystem로부터 읽어 세팅
            if (const FMissionBaseRow* MissionBaseRow = MissionSubsystem->GetMissionData(Choice.MissionType, Choice.MissionIndex))
            {
                State.Goal = MissionBaseRow->GoalCount;
            }

            ActiveStates.Add(State);
        }
    }

    OnActiveMissionCountChanged(); // <-- BindAll/UnbindAll 대신 이걸로
}

void UMissionManagerComponent::OnRep_Missions()
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Verbose, TEXT("[Client] OnRep_Missions: %d states"), ActiveStates.Num());
    for (const FMissionRuntimeState& S : ActiveStates)
    {
        UE_LOG(LogTemp, Verbose, TEXT("  - Type=%d Index=%d Cur=%d Goal=%d Completed=%s"),
            static_cast<int32>(S.MissionType),
            static_cast<int32>(S.MissionIndex),
            S.Current, S.Goal,
            S.bCompleted ? TEXT("true") : TEXT("false"));
    }
#endif

    // UI에 알림: 위젯/PC/HUD에서 이 BP 이벤트를 받아서 갱신하면 된다.
    BP_OnMissionStatesUpdated(ActiveStates);
}

void UMissionManagerComponent::HandleMonsterKilled(const FGameplayTagContainer& UnitTags) 
{ 
    AUTH_GUARD; 
    for (UMissionBase* Mission : ActiveMissions)
    {
        if (Mission)
        {
            Mission->NotifyMonsterKilled(UnitTags);
        }
    }
}

void UMissionManagerComponent::HandleItemCollected(const FGameplayTagContainer& ItemTags, int32 Amount)
{
    AUTH_GUARD;
    for (UMissionBase* Mission : ActiveMissions)
    {
        if (Mission)
        {
            Mission->NotifyItemCollected(ItemTags, Amount);
        }
    }
}

void UMissionManagerComponent::HandleItemUsed(const FGameplayTagContainer& ItemTags, int32 Amount)
{ 
    AUTH_GUARD; 
    for (UMissionBase* Mission : ActiveMissions)
    {
        if (Mission)
        {
            Mission->NotifyItemUsed(ItemTags, Amount);
		}
    }

}

void UMissionManagerComponent::HandleAggro(const FGameplayTagContainer& SourceTags) 
{ 
    AUTH_GUARD; 
    for (UMissionBase* Mission : ActiveMissions)
    {
        if (Mission)
        {
            Mission->NotifyAggroTriggered(SourceTags);
        }
    }
}

void UMissionManagerComponent::HandleInteracted(const FGameplayTagContainer& InteractTags) 
{ 
    AUTH_GUARD; 
    for (UMissionBase* Mission : ActiveMissions)
    {
        if (Mission)
        {
            Mission->NotifyInteracted(InteractTags);
		}
    }

}


void UMissionManagerComponent::HandleMissionProgress(EMissionType MissionType, uint8 MissionIndex, int32 Current, int32 Goal)
{
    for (FMissionRuntimeState& State : ActiveStates)
    {
        if (State.MissionType == MissionType && State.MissionIndex == MissionIndex)
        {
			State.Current = Current;
            State.Goal = Goal;
            break;
        }
    }

    OnRep_Missions(); 
}

void UMissionManagerComponent::SetProgress(uint8 Slot, int32 NewCurrent, int32 NewGoal, bool bForceRep)
{
    if (!ActiveStates.IsValidIndex(Slot)) return;
    ActiveStates[Slot].Current = NewCurrent;
    ActiveStates[Slot].Goal = NewGoal;
    if (bForceRep) { OnRep_Missions(); }
}

void UMissionManagerComponent::OnActiveMissionCountChanged()
{
    if (ActiveMissions.Num() > 0)
        WireHubEvents();
    else
    {
        UnbindAll();
    }
}

void UMissionManagerComponent::HandleMissionComplete(EMissionType Type, uint8 Index)
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
    if (!Hub) 
    {
        bEventsBound = false;
        return; 
    }

    auto SafeRemove = [](auto& Delegate, FDelegateHandle& Handle)
        {
            if (Handle.IsValid())
            {
                Delegate.Remove(Handle);
                Handle.Reset();
            }
        };

    SafeRemove(Hub->OnItemCollected, Handle_ItemCollected);
    SafeRemove(Hub->OnItemUsed, Handle_ItemUsed);
    SafeRemove(Hub->OnInteracted, Handle_Interacted);
    SafeRemove(Hub->OnMonsterKilled, Handle_MonsterKilled);
    SafeRemove(Hub->OnAggroTriggered, Handle_Aggro);

    bEventsBound = false;
}
void UMissionManagerComponent::WireHubEvents()
{
    if (bEventsBound) 
        return;

    if (!(GetOwner() && GetOwner()->HasAuthority()) || !Hub) return; 

    Handle_ItemCollected = Hub->OnItemCollected.AddUObject(this, &UMissionManagerComponent::HandleItemCollected);
    Handle_ItemUsed = Hub->OnItemUsed.AddUObject(this, &UMissionManagerComponent::HandleItemUsed);
    Handle_Interacted = Hub->OnInteracted.AddUObject(this, &UMissionManagerComponent::HandleInteracted);
    Handle_MonsterKilled = Hub->OnMonsterKilled.AddUObject(this, &UMissionManagerComponent::HandleMonsterKilled);
    Handle_Aggro = Hub->OnAggroTriggered.AddUObject(this, &UMissionManagerComponent::HandleAggro);

    bEventsBound = true;
    // OnItemUsed / OnInteracted / OnMonsterKilled / OnAggroTriggered 도 동일 패턴
}
