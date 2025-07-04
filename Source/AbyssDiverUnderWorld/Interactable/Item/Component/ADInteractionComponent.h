﻿#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/IADInteractable.h"
#include "ADInteractionComponent.generated.h"

class USphereComponent;
class UADInteractableComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UADInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UADInteractionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


#pragma region Method
public:
    UFUNCTION(Server, Reliable)
    void S_RequestInteract(AActor* TargetActor);
    void S_RequestInteract_Implementation(AActor* TargetActor);
    UFUNCTION(Server, Reliable)
    void S_RequestInteractHold(AActor* TargetActor);
    void S_RequestInteractHold_Implementation(AActor* TargetActor);
    UFUNCTION(Server, Reliable)
    void S_RequestInteractRelease();
    void S_RequestInteractRelease_Implementation();
    
    // Overlap 콜백 바인딩용 함수
    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    // 테스트 동안 OnInteractPressed 대신 사용할 함수
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void TryInteract();
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnInteractPressed();
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnInteractReleased();
    // Player가 E 키를 홀드 성공했을 때 호출할 함수
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnHoldComplete();

    void HandleInteractPressed(AActor* TargetActor);
    void HandleInteractReleased();

    // 실제 Focus 검사 함수
    void PerformFocusCheck();
    bool ComputeViewTrace(FVector& OutStart, FVector& OutEnd) const;
    UADInteractableComponent* PerformLineTrace(const FVector& Start, const FVector& End) const;
    void UpdateFocus(UADInteractableComponent* NewFocus);
    void ClearFocus();

    

    bool ShouldHighlight(const UADInteractableComponent* ADIC) const;
    bool IsLocallyControlled() const;

protected:
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty>& OutLifetimeProps) const override;

private:


#pragma endregion

#pragma region Variable
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHoldStart, AActor*, Target, float, Duration);
    UPROPERTY(BlueprintAssignable)
    FOnHoldStart OnHoldStart;
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHoldCancelSignature);
    UPROPERTY(BlueprintAssignable) 
    FOnHoldCancelSignature OnHoldCancel;
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
        FOnFocusInteractable, AActor*, FocusedActor, FString, Description);
    UPROPERTY(BlueprintAssignable) 
    FOnFocusInteractable OnFocus;
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFocusLost);
    UPROPERTY(BlueprintAssignable) 
    FOnFocusLost OnFocusEnd;


    UPROPERTY(Replicated)
    uint8 bIsInteractingStart : 1;
    UPROPERTY(Replicated)
    uint8 bHoldTriggered : 1;
    UPROPERTY(Replicated)
    uint8 bIsFocusing : 1;
    UPROPERTY(EditAnywhere)
    float HoldThreshold = 3.f;
    UPROPERTY()
    TSet<TObjectPtr<UADInteractableComponent>> NearbyInteractables;
    UPROPERTY()
    TObjectPtr<USphereComponent> RangeSphere = nullptr;
    UPROPERTY()
    TObjectPtr<UADInteractableComponent> FocusedInteractable = nullptr;

    
    FTimerHandle HoldTimerHandle;
    
    
    TWeakObjectPtr<AActor> HoldInstigator;
    TWeakObjectPtr<UADInteractableComponent> CachedHoldInteractable;
    TWeakObjectPtr<UADInteractableComponent> HeldInteractable;


protected:


private:
    FString CachedDesc;
    float InteractionRadius = 400.f;

#pragma endregion

#pragma region Getter, Setteer
public:
    UFUNCTION(BlueprintPure, Category = "Interaction")
    UADInteractableComponent* GetFocusedInteractable() const
    {
        return FocusedInteractable;
    }
    USphereComponent* GetRangeSphere() const { return RangeSphere; }

#pragma endregion
};
