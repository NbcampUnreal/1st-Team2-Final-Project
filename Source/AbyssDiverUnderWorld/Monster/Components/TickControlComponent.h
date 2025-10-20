#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"

#include "TickControlComponent.generated.h"

// 플레이어별 상태 캐싱
struct FPlayerState
{
    FVector Location;
    FVector CameraLocation;
    FVector CameraForward;
    uint8 bIsValid : 1 = false;
    uint8 bIsLocalPlayer : 1 = false;  // 로컬 플레이어 구분용
};

// 가시성 상태 열거형
enum class EVisibilityState : uint8
{
    FullyVisible,
    InViewOccluded,
    OutOfView
};

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

    // 등록 함수
    // 틱 관리할 컴포넌트 등록
    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void RegisterComponent(UActorComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void UnregisterComponent(UActorComponent* Component);

    // 틱 관리할 액터 등록
    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void RegisterNewActor(AActor* NewActor);

    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void UnregisterNewActor(AActor* NewActor);

    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void ClearAllComponentsAndActors();

    // 장애물 취급 무시용 액터 등록
    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void AddIgnoreActor(AActor* IgnoreActor);

    UFUNCTION(BlueprintCallable, Category = "TickControlComponentSettings|Tick Control")
    void RemoveIgnoreActor(AActor* IgnoreActor);

private:

    void InitializeNetworkInfo();
    void UpdateTickRate();
    void UpdatePlayerStates();

    void SetComponentTickRate(UActorComponent* Component, float TickInterval);
    void SetActorTickRate(AActor* RegisteredActor, float TickInterval);
    // 컴포넌트 틱을 비활성화했다가 다시 활성화하여 즉시 적용하여 틱 즉시 재시작
    void ForceComponentTickReset(UActorComponent* Component);
    void ForceActorTickReset(AActor* RegisteredActor);

protected:

    // 제어할 컴포넌트들
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "TickControlComponentSettings|Targets", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UActorComponent>> TargetComponents;

    // 제어할 액터들
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "TickControlComponentSettings|Targets", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AActor>> TargetActors;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "TickControlComponentSettings", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<AActor>> IgnoreActors;

    // 플레이어 캐시 간격 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Player State Cache", meta = (AllowPrivateAccess = "true"))
    float PlayerStateCacheInterval = 1.0f;

    // 사용 여부 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Enable Settings", meta = (AllowPrivateAccess = "true"))
	uint8 bEnableTickControl : 1 = true;

    // 거리별 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Distance Settings", meta = (AllowPrivateAccess = "true"))
    float NearDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Distance Settings", meta = (AllowPrivateAccess = "true"))
    float MediumDistance = 7000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Distance Settings", meta = (AllowPrivateAccess = "true"))
    float FarDistance = 10000.0f;

    // 시야 내 틱 간격 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|In View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float InView_NearTickInterval = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|In View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float InView_MediumTickInterval = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|In View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float InView_FarTickInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|In View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float InView_VeryFarTickInterval = 2.0f;

    // 가려진 오브젝트 틱 간격 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Occluded Tick Rates", meta = (AllowPrivateAccess = "true"))
    float Occluded_NearTickInterval = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Occluded Tick Rates", meta = (AllowPrivateAccess = "true"))
    float Occluded_MediumTickInterval = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Occluded Tick Rates", meta = (AllowPrivateAccess = "true"))
    float Occluded_FarTickInterval = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Occluded Tick Rates", meta = (AllowPrivateAccess = "true"))
    float Occluded_VeryFarTickInterval = 1000.0f;

    // 시야 밖 틱 간격 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Out of View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float OutOfView_NearTickInterval = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Out of View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float OutOfView_MediumTickInterval = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Out of View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float OutOfView_FarTickInterval = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Out of View Tick Rates", meta = (AllowPrivateAccess = "true"))
    float OutOfView_VeryFarTickInterval = 3000.0f;

    // 시야 판정 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|View Settings", meta = (AllowPrivateAccess = "true"))
    float ViewAngleThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|View Settings", meta = (AllowPrivateAccess = "true"))
    float ViewAngleBuffer = 15.0f;

    // 가시성 체크 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Visibility Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bEnableVisibilityCheck : 1 = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Visibility Settings", meta = (AllowPrivateAccess = "true"))
    float VisibilityCheckDistance = 12000.0f;

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
    float UpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Update Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bEnableDistanceBasedTick : 1 = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Update Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bEnableViewBasedTick : 1 = true;

    // 클라이언트 전용 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Client Settings", meta = (AllowPrivateAccess = "true"))
    uint8 bClientOnlyCheckLocalPlayer : 1 = true;  // 클라이언트는 로컬 플레이어만 체크

    // 프레임 분산 설정

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickControlComponentSettings|Performance Settings", meta = (AllowPrivateAccess = "true"))
    //uint8 bEnableFrameDistribution : 1 = true;

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

    TArray<FPlayerState> CachedPlayerStates;
    float LastPlayerStateCacheTime = 0.0f;

private:

    float GetClosestPlayerDistance();
    EVisibilityState GetBestVisibilityState();
    bool IsInAnyPlayerView();
    bool IsVisibleToAnyPlayer();
    bool ShouldCheckAllPlayers() const;

    // 개별 플레이어 체크 함수들

    bool IsInPlayerView(const FPlayerState& PlayerState);
    bool IsVisibleToPlayer(const FPlayerState& PlayerState);
    float GetDistanceToPlayer(const FPlayerState& PlayerState);
};