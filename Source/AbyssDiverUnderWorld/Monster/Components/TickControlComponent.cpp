#include "TickControlComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UTickControlComponent::UTickControlComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // 네트워크 설정 - 이 컴포넌트 자체는 동기화하지 않음
    SetIsReplicated(false);
}

void UTickControlComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bEnableTickControl == false)
    {
        PrimaryComponentTick.bCanEverTick = false;
        SetComponentTickEnabled(false);
        PrimaryComponentTick.TickInterval = FLT_MAX;
        return;
    }

    // 네트워크 정보 초기화
    InitializeNetworkInfo();

    // 기본 가시성 체크 오브젝트 타입 설정
    if (VisibilityBlockingObjects.Num() == 0)
    {
        VisibilityBlockingObjects.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    }

    // 플레이어 상태 배열 초기화
    CachedPlayerStates.SetNum(MaxPlayerCount);

    // 초기 틱 레이트 설정
    if (TargetComponents.Num() > 0 && (bEnableDistanceBasedTick || bEnableViewBasedTick))
    {
        UpdatePlayerStates();
        UpdateTickRate();
    }
}

void UTickControlComponent::InitializeNetworkInfo()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    bIsServer = World->GetAuthGameMode() != nullptr;
    
    if (GetOwner())
    {
        bHasAuthority = GetOwner()->HasAuthority();
    }
}

void UTickControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableTickControl == false)
    {
        return;
    }

    if (TargetComponents.Num() == 0 || (!bEnableDistanceBasedTick && !bEnableViewBasedTick))
    {
        return;
    }

    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdatePlayerStates();

        if (bEnableFrameDistribution && TargetComponents.Num() > MaxChecksPerFrame)
        {
            // 프레임 분산 처리
            int32 ComponentsToCheck = FMath::Min(MaxChecksPerFrame, TargetComponents.Num());
            for (int32 i = 0; i < ComponentsToCheck; ++i)
            {
                int32 Index = (CurrentCheckIndex + i) % TargetComponents.Num();
                if (TargetComponents.IsValidIndex(Index) && TargetComponents[Index])
                {
                    UpdateTickRate();  // 개별 컴포넌트별로는 구현하지 않고 전체 적용
                    break;
                }
            }

            CurrentCheckIndex = (CurrentCheckIndex + ComponentsToCheck) % TargetComponents.Num();
        }
        else
        {
            UpdateTickRate();
        }

        LastUpdateTime = 0.0f;
    }
}

bool UTickControlComponent::ShouldCheckAllPlayers() const
{
    if (bIsServer && bServerAlwaysCheckAllPlayers)
    {
        return true;  // 서버는 항상 모든 플레이어 체크
    }
    
    if (bIsServer == false && bClientOnlyCheckLocalPlayer)
    {
        return false;  // 클라이언트는 로컬 플레이어만
    }
    
    return bCheckAllPlayers;
}

void UTickControlComponent::RegisterComponent(UActorComponent* Component)
{
    if (Component && Component != this && !TargetComponents.Contains(Component))
    {
        TargetComponents.Add(Component);
    }
}

void UTickControlComponent::UnregisterComponent(UActorComponent* Component)
{
    TargetComponents.Remove(Component);
}

void UTickControlComponent::ClearAllComponents()
{
    TargetComponents.Reset();
}

void UTickControlComponent::UpdatePlayerStates()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastPlayerStateCacheTime < PlayerStateCacheInterval)
    {
        return;  // 캐시된 상태 사용
    }

    LastPlayerStateCacheTime = CurrentTime;

    // 로컬 플레이어 컨트롤러 찾기 (클라이언트용)
    APlayerController* LocalPC = World->GetFirstPlayerController();
    
    // 클라이언트에서 로컬 플레이어만 체크하는 경우 최적화
    bool bCheckOnlyLocal = (bIsServer == false && bClientOnlyCheckLocalPlayer);

    for (int32 i = 0; i < MaxPlayerCount; ++i)
    {
        FPlayerState& PlayerState = CachedPlayerStates[i];
        PlayerState.bIsValid = false;
        PlayerState.bIsLocalPlayer = false;

        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, i);
        if (PlayerPawn == nullptr)
        {
            continue;
        }

        APlayerController* PC = UGameplayStatics::GetPlayerController(World, i);
        if (PC == nullptr)
        {
            continue;
        }

        if (bCheckOnlyLocal && PC != LocalPC)
        {
            return;  // 이미 로컬 플레이어 상태만 필요하므로 캐시 불필요
        }

        PlayerState.Location = PlayerPawn->GetActorLocation();
        PlayerState.bIsLocalPlayer = (PC == LocalPC);

        FVector CameraLocation;
        FRotator CameraRotation;
        PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

        PlayerState.CameraLocation = CameraLocation;
        PlayerState.CameraForward = CameraRotation.Vector();
        PlayerState.bIsValid = true;
    }
}

void UTickControlComponent::UpdateTickRate()
{
    if (TargetComponents.Num() == 0)
    {
        return;
    }

    // 거리 계산
    CurrentDistance = GetClosestPlayerDistance();

    // 가시성 상태 확인
    EVisibilityState VisibilityState = GetBestVisibilityState();

    // 거리별 카테고리와 가시성 상태에 따른 틱 간격 결정
    float TickInterval = 0.0f;

    if (VisibilityState == EVisibilityState::FullyVisible)
    {
        if (CurrentDistance <= NearDistance)
        {
            TickInterval = InView_NearTickInterval;
        }
        else if (CurrentDistance <= MediumDistance)
        {
            TickInterval = InView_MediumTickInterval;
        }
        else if (CurrentDistance <= FarDistance)
        {
            TickInterval = InView_FarTickInterval;
        }
        else
        {
            TickInterval = InView_VeryFarTickInterval;
        }
    }
    else if (VisibilityState == EVisibilityState::InViewOccluded)
    {
        if (CurrentDistance <= NearDistance)
        {
            TickInterval = Occluded_NearTickInterval;
        }
        else if (CurrentDistance <= MediumDistance)
        {
            TickInterval = Occluded_MediumTickInterval;
        }
        else if (CurrentDistance <= FarDistance)
        {
            TickInterval = Occluded_FarTickInterval;
        }
        else
        {
            TickInterval = Occluded_VeryFarTickInterval;
        }
    }
    else // OutOfView
    {
        if (CurrentDistance <= NearDistance)
        {
            TickInterval = OutOfView_NearTickInterval;
        }
        else if (CurrentDistance <= MediumDistance)
        {
            TickInterval = OutOfView_MediumTickInterval;
        }
        else if (CurrentDistance <= FarDistance)
        {
            TickInterval = OutOfView_FarTickInterval;
        }
        else
        {
            TickInterval = OutOfView_VeryFarTickInterval;
        }
    }

    // 모든 등록된 컴포넌트에 틱 레이트 적용
    for (UActorComponent* Component : TargetComponents)
    {
        SetComponentTickRate(Component, TickInterval);
    }

    // 디버그 정보 출력
    if (bShowDebugInfo && GetOwner())
    {
        FString StateString;
        switch (VisibilityState)
        {
        case EVisibilityState::FullyVisible:
            StateString = TEXT("VISIBLE");
            break;
        case EVisibilityState::InViewOccluded:
            StateString = TEXT("OCCLUDED");
            break;
        case EVisibilityState::OutOfView:
            StateString = TEXT("OUT_OF_VIEW");
            break;
        }

        FString NetworkRole = bIsServer ? TEXT("SERVER") : TEXT("CLIENT");
        FString Authority = bHasAuthority ? TEXT("AUTH") : TEXT("NO_AUTH");

        FString DebugText = FString::Printf(
            TEXT("%s [%s_%s] - Distance: %.0f, State: %s, Tick: %.3f, Components: %d"),
            *GetOwner()->GetName(),
            *NetworkRole,
            *Authority,
            CurrentDistance,
            *StateString,
            TickInterval,
            TargetComponents.Num()
        );

        FColor DebugColor = FColor::Red;
        switch (VisibilityState)
        {
        case EVisibilityState::FullyVisible:
            DebugColor = bIsServer ? FColor::Green : FColor::Cyan;
            break;
        case EVisibilityState::InViewOccluded:
            DebugColor = bIsServer ? FColor::Orange : FColor::Yellow;
            break;
        case EVisibilityState::OutOfView:
            DebugColor = bIsServer ? FColor::Red : FColor::Magenta;
            break;
        }

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                (int32)GetOwner()->GetUniqueID(),
                UpdateInterval + 0.1f,
                DebugColor,
                DebugText
            );
        }
    }
}

float UTickControlComponent::GetClosestPlayerDistance()
{
    if (GetOwner() == nullptr)
    {
        return 0.0f;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float ClosestDistance = FLT_MAX;

    bool bCheckAll = ShouldCheckAllPlayers();

    for (const FPlayerState& PlayerState : CachedPlayerStates)
    {
        if (PlayerState.bIsValid)
        {
            float Distance = FVector::Dist(OwnerLocation, PlayerState.Location);
            ClosestDistance = FMath::Min(ClosestDistance, Distance);

            if (bCheckAll == false)
            {
                break;  // 첫 번째 유효한 플레이어만 체크
            }
        }
    }

    return ClosestDistance == FLT_MAX ? 0.0f : ClosestDistance;
}

UTickControlComponent::EVisibilityState UTickControlComponent::GetBestVisibilityState()
{
    if (bEnableViewBasedTick == false)
    {
        return EVisibilityState::FullyVisible;  // 시야 체크 비활성화시 항상 보이는 것으로 처리
    }

    bool bInAnyView = IsInAnyPlayerView();

    if (bInAnyView == false)
    {
        return EVisibilityState::OutOfView;
    }

    if (bEnableVisibilityCheck)
    {
        bool bVisibleToAny = IsVisibleToAnyPlayer();
        return bVisibleToAny ? EVisibilityState::FullyVisible : EVisibilityState::InViewOccluded;
    }

    return EVisibilityState::FullyVisible;
}

bool UTickControlComponent::IsInAnyPlayerView()
{
    bool bCheckAll = ShouldCheckAllPlayers();

    for (const FPlayerState& PlayerState : CachedPlayerStates)
    {
        if (PlayerState.bIsValid && IsInPlayerView(PlayerState))
        {
            return true;
        }

        if (bCheckAll == false)
        {
            break;  // 첫 번째 플레이어만 체크
        }
    }

    return false;
}

bool UTickControlComponent::IsVisibleToAnyPlayer()
{
    if (CurrentDistance > VisibilityCheckDistance)
    {
        return false;
    }

    bool bCheckAll = ShouldCheckAllPlayers();

    for (const FPlayerState& PlayerState : CachedPlayerStates)
    {
        if (PlayerState.bIsValid && IsVisibleToPlayer(PlayerState))
        {
            return true;
        }

        if (bCheckAll == false)
        {
            break;  // 첫 번째 플레이어만 체크
        }
    }

    return false;
}

bool UTickControlComponent::IsInPlayerView(const FPlayerState& PlayerState)
{
    if (GetOwner() == nullptr || PlayerState.CameraForward.IsZero())
    {
        return true;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DirectionToObject = (OwnerLocation - PlayerState.CameraLocation).GetSafeNormal();

    float DotProduct = FVector::DotProduct(PlayerState.CameraForward, DirectionToObject);
    float AngleInRadians = FMath::Acos(DotProduct);
    float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);

    // 히스테리시스 적용
    float ThresholdAngle = ViewAngleThreshold;
    if (bWasInViewLastFrame == false)
    {
        ThresholdAngle += ViewAngleBuffer;
    }
    else
    {
        ThresholdAngle -= ViewAngleBuffer;
    }

    return AngleInDegrees <= ThresholdAngle;
}

bool UTickControlComponent::IsVisibleToPlayer(const FPlayerState& PlayerState)
{
    if (GetWorld() == nullptr || GetOwner() == nullptr)
    {
        return true;
    }

    FVector TraceStart = PlayerState.CameraLocation;
    FVector TraceEnd = GetOwner()->GetActorLocation();
    TraceEnd.Z += 100.0f;  // 몸통 중간 체크

    FHitResult HitResult;
    bool bHit = UKismetSystemLibrary::LineTraceSingleForObjects(
        GetWorld(),
        TraceStart,
        TraceEnd,
        VisibilityBlockingObjects,
        false,
        TArray<AActor*>({ GetOwner() }),
        bShowDebugInfo ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
        HitResult,
        true
    );

    return bHit == false;
}

float UTickControlComponent::GetDistanceToPlayer(const FPlayerState& PlayerState)
{
    if (GetOwner() == nullptr)
    {
        return 0.0f;
    }

    return FVector::Dist(GetOwner()->GetActorLocation(), PlayerState.Location);
}

void UTickControlComponent::SetComponentTickRate(UActorComponent* Component, float TickInterval)
{
    if (Component == nullptr)
    {
        return;
    }

    if (TickInterval <= 0.0f)
    {
        Component->PrimaryComponentTick.bCanEverTick = true;
        Component->PrimaryComponentTick.TickInterval = 0.0f;
    }
    else
    {
        Component->PrimaryComponentTick.bCanEverTick = true;
        Component->PrimaryComponentTick.TickInterval = TickInterval;
    }
}