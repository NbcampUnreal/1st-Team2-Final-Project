#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"

#include "TickControlComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API UTickControlComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UTickControlComponent();

protected:

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // 컴포넌트 등록 함수

    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void RegisterComponent(UActorComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void UnregisterComponent(UActorComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void ClearAllComponents();

protected:

    // 제어할 컴포넌트들
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Target Components", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UActorComponent>> TargetComponents;

    // 플레이어 캐시 간격 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Player State Cache", meta = (AllowPrivateAccess = "true"))
    float PlayerStateCacheInterval = 1.0f;

    // 사용 여부 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Enable Settings", meta = (AllowPrivateAccess = "true"))
	uint8 bEnableTickControl : 1 = true;

    // 거리별 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Distance Settings", meta = (AllowPrivateAccess = "true"))
    float NearDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Distance Settings", meta = (AllowPrivateAccess = "true"))
    float MediumDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Distance Settings", meta = (AllowPrivateAccess = "true"))
    float FarDistance = 3000.0f;

    // 시야 내 틱 간격 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|In View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float InView_NearTickInterval = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|In View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float InView_MediumTickInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|In View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float InView_FarTickInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|In View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float InView_VeryFarTickInterval = 2.0f;

    // 가려진 오브젝트 틱 간격 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Occluded Tick Rates", meta = (AllowPrivateAccess = "true"))
    float Occluded_NearTickInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Occluded Tick Rates", meta = (AllowPrivateAccess = "true"))
    float Occluded_MediumTickInterval = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Occluded Tick Rates", meta = (AllowPrivateAccess = "true"))
    float Occluded_FarTickInterval = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Occluded Tick Rates", meta = (AllowPrivateAccess = "true"))
    float Occluded_VeryFarTickInterval = 2.5f;

    // 시야 밖 틱 간격 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Out of View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float OutOfView_NearTickInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Out of View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float OutOfView_MediumTickInterval = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Out of View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float OutOfView_FarTickInterval = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Out of View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float OutOfView_VeryFarTickInterval = 2.5f;

    // 시야 판정 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|View Settings", meta = (AllowPrivateAccess = "true"))
    float ViewAngleThreshold = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|View Settings", meta = (AllowPrivateAccess = "true"))
    float ViewAngleBuffer = 15.0f;

    // 가시성 체크 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Visibility Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bEnableVisibilityCheck : 1 = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Visibility Settings", meta = (AllowPrivateAccess = "true"))
    float VisibilityCheckDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Visibility Settings", meta = (AllowPrivateAccess = "true"))
    TArray<TEnumAsByte<EObjectTypeQuery>> VisibilityBlockingObjects;

    // 멀티플레이어 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Multiplayer Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxPlayerCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Multiplayer Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bCheckAllPlayers : 1 = false;  // true: 모든 플레이어 체크, false: 가장 가까운 플레이어만

    // 서버 전용 설정 (리슨서버용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Server Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bServerAlwaysCheckAllPlayers : 1 = true;  // 서버는 항상 모든 플레이어 체크

    // 업데이트 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Update Settings", meta = (AllowPrivateAccess = "true"))
    float UpdateInterval = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Update Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bEnableDistanceBasedTick : 1 = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Update Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bEnableViewBasedTick : 1 = true;

    // 클라이언트 전용 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Client Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bClientOnlyCheckLocalPlayer : 1 = true;  // 클라이언트는 로컬 플레이어만 체크

    // 프레임 분산 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Performance Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bEnableFrameDistribution : 1 = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Performance Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxChecksPerFrame = 5;  // 한 프레임당 최대 체크 수

    // 디버그

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Debug", meta = (AllowPrivateAccess = "true"))
    uint8 bShowDebugInfo : 1 = false;

private:

    float LastUpdateTime = 0.0f;
    uint8 bWasInViewLastFrame : 1 = true;
    uint8 bWasVisibleLastFrame : 1 = true;
    float CurrentDistance = 0.0f;
    int32 CurrentCheckIndex = 0;  // 프레임 분산용 인덱스

    // 네트워크 관련
    uint8 bIsServer : 1 = false;
    uint8 bHasAuthority : 1 = false;

    // 플레이어별 상태 캐싱
    struct FPlayerState
    {
        FVector Location;
        FVector CameraLocation;
        FVector CameraForward;
        uint8 bIsValid : 1 = false;
        uint8 bIsLocalPlayer : 1 = false;  // 로컬 플레이어 구분용
    };

    TArray<FPlayerState> CachedPlayerStates;
    float LastPlayerStateCacheTime = 0.0f;
    
    // 가시성 상태 열거형
    enum class EVisibilityState : uint8
    {
        FullyVisible,
        InViewOccluded,
        OutOfView
    };

    void InitializeNetworkInfo();
    void UpdateTickRate();
    void UpdatePlayerStates();
    float GetClosestPlayerDistance();
    EVisibilityState GetBestVisibilityState();
    bool IsInAnyPlayerView();
    bool IsVisibleToAnyPlayer();
    void SetComponentTickRate(UActorComponent* Component, float TickInterval);
    bool ShouldCheckAllPlayers() const;

    // 개별 플레이어 체크 함수들

    bool IsInPlayerView(const FPlayerState& PlayerState);
    bool IsVisibleToPlayer(const FPlayerState& PlayerState);
    float GetDistanceToPlayer(const FPlayerState& PlayerState);
};