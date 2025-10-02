#include "MissionManagerComponent.h"
#include "MissionEventHubComponent.h"
#include "Framework/ADInGameState.h"
#include "Subsystems/MissionSubsystem.h"
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
#define AUTH_GUARD_PTR if (!(GetOwner() && GetOwner()->HasAuthority())) return nullptr;



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

void UMissionManagerComponent::Multicast_NotifyMissionCompleted_Implementation(EMissionType MissionType, int32 MissionIndex)
{
    OnMissionCompletedUI.Broadcast(MissionType, MissionIndex);
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

void UMissionManagerComponent::BuildMissionStateSnapshot(TArray<FMissionRuntimeState>& Out) const
{
    Out.Reset();

    const bool bServer = (GetOwner() && GetOwner()->HasAuthority());

    if (bServer)
    {
        // 서버: 실제 미션 객체에서 현재 상태를 조립
        for (const UMissionBase* M : ActiveMissions)
        {
            if (!M) continue;

            FMissionRuntimeState S;
            S.MissionType = M->GetMissionType();
            S.MissionIndex = M->GetMissionIndex();
            S.Current = M->GetCurrentCount();
            S.Goal = M->GetGoalCount();
            S.bCompleted = M->IsCompleted();
            Out.Add(S);
        }
    }
    else
    {
        // 클라: 복제된 요약을 그대로 사용(빠르고 안전)
        Out.Append(ActiveStates);
    }
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

    // 1) 블루프린트 구현형 이벤트(선택)
    BP_OnMissionStatesUpdated(ActiveStates);
    // 2) 멀티캐스트(UMG/C++ 위젯에서 바인딩)
    OnMissionStatesUpdated.Broadcast(ActiveStates);
}

void UMissionManagerComponent::HandleMonsterKilled(const FGameplayTagContainer& UnitTags) 
{ 
    AUTH_GUARD; 
    for (UMissionBase* Mission : ActiveMissions)
    {
        if (Mission && Mission->GetMissionType() == EMissionType::KillMonster)
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
        if (Mission && Mission->GetMissionType() == EMissionType::ItemCollection)
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
        if (Mission && Mission->GetMissionType() == EMissionType::ItemUse)
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
        if (Mission && Mission->GetMissionType() == EMissionType::AggroTrigger)
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
        if (Mission && Mission->GetMissionType() == EMissionType::Interaction)
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

void UMissionManagerComponent::HandleMissionComplete(EMissionType MissionType, uint8 MissionIndex)
{
    for (FMissionRuntimeState& S : ActiveStates)
    {
        if (S.MissionType == MissionType && S.MissionIndex == MissionIndex)
        {
            Multicast_NotifyMissionCompleted(MissionType, static_cast<int32>(MissionIndex));
        }
    }
}

void UMissionManagerComponent::GatherCompletedMissions(TArray<FCompletedMissionInfo>& Out) const
{
    Out.Reset();
    for (const UMissionBase* M : ActiveMissions)
    {
        if (M && M->IsCompleted())
        {
            FCompletedMissionInfo Info;
            Info.MissionType = M->GetMissionType();
            Info.MissionIndex = M->GetMissionIndex();
            // 보상이 있다면 추가하기
            Out.Add(Info);
        }
    }
}

void UMissionManagerComponent::SetProgress(uint8 Slot, int32 NewCurrent, int32 NewGoal, bool bForceRep)
{
    if (!ActiveStates.IsValidIndex(Slot)) return;
    ActiveStates[Slot].Current = NewCurrent;
    ActiveStates[Slot].Goal = NewGoal;
    if (bForceRep) { OnRep_Missions(); }
}

UClass* UMissionManagerComponent::PickMissiopnClass(EMissionType Type)
{
    switch (Type)
    {
    case EMissionType::KillMonster:     return UKillMonsterMission::StaticClass();
    case EMissionType::ItemCollection:  return UItemCollectionMission::StaticClass();
    case EMissionType::ItemUse:         return UItemUseMission::StaticClass();
    case EMissionType::AggroTrigger:    return UAggroTriggerMission::StaticClass();
    case EMissionType::Interaction:     return UInteractionMission::StaticClass();
    default:                            return nullptr;
    }
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

UMissionBase* UMissionManagerComponent::CreateAndInitMission(const FMissionData& Choice)
{
    AUTH_GUARD_PTR;
	if (!MissionSubsystem) return nullptr;

    // DT Row 조회
    const FMissionBaseRow* BaseRow = MissionSubsystem->GetMissionData(Choice.MissionType, Choice.MissionIndex);
	if (!BaseRow) 
        return nullptr;

    //Class 결정
	UClass* MissionClass = PickMissiopnClass(Choice.MissionType);
    if (!MissionClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[Mission] Unsupported MissionType: %d"), (int32)Choice.MissionType);
        return nullptr;
    }
    //생성
	UMissionBase* Mission = NewObject<UMissionBase>(this, MissionClass);

    // 4) Row 사본 + Bake + Initialize
    switch (Choice.MissionType)
    {
    case EMissionType::KillMonster:
    {
        FKillMonsterMissionRow Row = *static_cast<const FKillMonsterMissionRow*>(BaseRow);

		Row.BakeTags();

        FKillMissionInitParams Params(
            Row.MissionType,            // EMissionType
            Row.MissionName,            // FString
            Row.MissionDescription,     // FString
            Row.ConditionType,          // EMissionConditionType
            Row.GoalCount,              // int32
            Row.ExtraValues,            // TArray<int32>
            Row.bShouldCompleteInstantly,     // bool
            Row.bUseQuery,              // bool
            Row.TargetUnitIdTag,        // FGameplayTag
            Row.TargetUnitTypeTag,      // FGameplayTag
            Row.TargetQuery,        // FGameplayTagQuery
            Row.NeededSimultaneousKillCount, // uint8
            Row.KillInterval                  // float
        );

        if (UKillMonsterMission* KillMission = Cast<UKillMonsterMission>(Mission))
        {
            KillMission->InitMission(Params, static_cast<EKillMonsterMission>(Choice.MissionIndex));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Mission] MissionClass is not UKillMonsterMission"));
            return nullptr;
        }
        break;
    }

    case EMissionType::ItemCollection:
    {
        FItemCollectMissionRow Row = *static_cast<const FItemCollectMissionRow*>(BaseRow);
        Row.BakeTags();

        FItemCollectMissionInitParams Params(
            Row.MissionType, 
            Row.MissionName,
            Row.MissionDescription,
            Row.ConditionType,
            Row.GoalCount, 
            Row.ExtraValues,
            Row.bShouldCompleteInstantly,
            Row.bUseQuery,
            Row.TargetItemIdTag,
            Row.TargetItemTypeTag,
            Row.TargetQuery
        );

        if (UItemCollectionMission* ItemMission = Cast<UItemCollectionMission>(Mission))
        {
            ItemMission->InitMission(Params, static_cast<EItemCollectMission>(Choice.MissionIndex));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Mission] MissionClass is not UItemCollectionMission"));
            return nullptr;
		}
        break;
    }

    case EMissionType::ItemUse:
    {
        FItemUseMissionRow Row = *static_cast<const FItemUseMissionRow*>(BaseRow);
        Row.BakeTags();

        FItemUseMissionInitParams Params(
            Row.MissionType, 
            Row.MissionName, 
            Row.MissionDescription,
            Row.ConditionType, 
            Row.GoalCount,
            Row.ExtraValues,
            Row.bShouldCompleteInstantly,
            Row.bUseQuery,
            Row.TargetItemIdTag,
            Row.TargetItemTypeTag,
            Row.TargetQuery
        );

        if (UItemUseMission* ItemUseMission = Cast<UItemUseMission>(Mission))
        {
            ItemUseMission->InitMission(Params, static_cast<EItemUseMission>(Choice.MissionIndex));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Mission] MissionClass is not UItemUseMission"));
            return nullptr;
		}
        break;
    }

    case EMissionType::AggroTrigger:
    {
        FAggroTriggerMissionRow Row = *static_cast<const FAggroTriggerMissionRow*>(BaseRow);

        Row.BakeTags();

        FAggroMissionInitParams Params(
            Row.MissionType,
            Row.MissionName, 
            Row.MissionDescription,
            Row.ConditionType,
            Row.GoalCount,
            Row.ExtraValues,
            Row.bShouldCompleteInstantly,
            Row.bUseQuery,
            Row.TargetSourceUnitIdTag,     // 선택 사용(필요 없으면 기본값)
            Row.TargetSourceUnitTypeTag,
            Row.TargetQuery
        );

        if (UAggroTriggerMission* AggroMission = Cast<UAggroTriggerMission>(Mission))
        {
            AggroMission->InitMission(Params, static_cast<EAggroTriggerMission>(Choice.MissionIndex));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Mission] MissionClass is not UAggroTriggerMission"));
            return nullptr;
        }
        break;
    }

    case EMissionType::Interaction:
    {
        FInteractionMissionRow Row = *static_cast<const FInteractionMissionRow*>(BaseRow);

        Row.BakeTags();

        FInteractiontMissionInitParams Params(
            Row.MissionType, 
            Row.MissionName,
            Row.MissionDescription,
            Row.ConditionType, 
            Row.GoalCount,
            Row.ExtraValues, 
            Row.bShouldCompleteInstantly,
            Row.bUseQuery,
            Row.TargetInteractUnitIdTag,   // 선택 사용
            Row.TargetInteractTypeTag,
            Row.TargetQuery
        );

        if (UInteractionMission* InteractionMission = Cast<UInteractionMission>(Mission))
        {
            InteractionMission->InitMission(Params, static_cast<EInteractionMission>(Choice.MissionIndex));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Mission] MissionClass is not UInteractionMission"));
        }
        break;
    }

    default:
        checkNoEntry();
        return nullptr;
    }

    // 5) 보관 (UPROPERTY로 관리되는 컨테이너여야 GC 안전)
    ActiveMissions.Add(Mission);

    return Mission;
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
