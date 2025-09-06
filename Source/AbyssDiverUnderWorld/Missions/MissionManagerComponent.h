#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/MissionData.h"      
#include "MissionManagerComponent.generated.h"

class UMissionBase;
class UMissionEventHubComponent;
class UMissionSubsystem; // DT/카탈로그 조회용 (기존 서브시스템)

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
    // ① 클라→서버: 태블릿에서 선택된 미션 전달
    UFUNCTION(Server, Reliable)
    void S_SubmitMissions(const TArray<FMissionData>& Selected);
	void S_SubmitMissions_Implementation(const TArray<FMissionData>& Selected);

    // ② 클라 HUD가 구독할 얇은 복제 상태
    UPROPERTY(ReplicatedUsing = OnRep_Missions)
    TArray<FMissionRuntimeState> ActiveStates;

    UFUNCTION()
    void OnRep_Missions(); // HUD/Tablet 갱신 트리거
private:
    // 서버에만 존재하는 실제 미션 객체들
    UPROPERTY(Transient)
    TArray<TObjectPtr<UMissionBase>> ActiveMissions;

    // 허브(이벤트 집결지, GameState에 붙음)
    UPROPERTY()
    TObjectPtr<UMissionEventHubComponent> Hub;

    // 데이터테이블/카탈로그 조회용 (기존 서브시스템)
    UPROPERTY()
    TObjectPtr<UMissionSubsystem> MissionSubsystem;

    // 기존 Subsystem의 switch-생성-Init 로직을 이식할 자리
    UMissionBase* CreateAndInitMission(const FMissionData& Choice);

    // 허브 구독/해제 + 이벤트 라우팅
    void BindAll();
    void UnbindAll();
    void WireHubEvents();

    // 완료 콜백(UMissionBase의 싱글캐스트 델리게이트가 호출)
    void HandleMissionComplete(const EMissionType& Type, const uint8& Index);

    // 진행값 갱신 헬퍼
    void SetProgress(uint8 Slot, int32 NewCurrent, int32 NewGoal, bool bForceRep = false);


		
};
