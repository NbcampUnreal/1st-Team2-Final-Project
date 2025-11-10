#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "TargetIndicatorManager.generated.h"

class AIndicatingTarget;
class UTargetIndicatorWidget;
class UBillboardComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATargetIndicatorManager : public AActor
{
	GENERATED_BODY()
	
public:	

	ATargetIndicatorManager();

protected:

	virtual void BeginPlay() override;

#pragma region Methods

public:

	void SkipCurrentTarget();

	// 만약 현재 타겟이 DroneSeller일 경우 Drone이 활성화 된 상태일 때면 스킵한다.
	// 나중에 리펙토링 필요.. 지스타를 위한 임시 함수
	void SkipTargetIfDroneActivated();

private:

	void OnIndicatingTargetOverlapped(int32 TargetOrder);
	void TryActivateNextTarget();

	void SkipTarget(int32 TargetOrderForSkip);


#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TargetIndicatorManagerSettings")
	uint8 bIsActivated : 1;

	UPROPERTY(VisibleAnywhere, Category = "TargetIndicatorManager")
	TObjectPtr<UBillboardComponent> BillboardSprite;

	UPROPERTY(EditDefaultsOnly, Category = "TargetIndicatorManagerSettings")
	TSubclassOf<UTargetIndicatorWidget> TargetIndicatorWidgetClass;

	UPROPERTY()
	TObjectPtr<UTargetIndicatorWidget> TargetIndicatorWidgetInstance;

private:

	// Order의 역순으로 정렬, 뒤에서부터 사용
	UPROPERTY()
	TArray<TObjectPtr<AIndicatingTarget>> TargetArrayByReverseOrder;

	//UPROPERTY()
	//TObjectPtr<AIndicatingTarget> CurrentTarget;

#pragma endregion

#pragma region Getters / Setters

public:

	UFUNCTION(BlueprintCallable, Category = "TargetIndicatorManager")
	void SetActive(bool bShouldActivate);
	bool IsActivated();

	bool TryGetTargetLocation(FVector& OutLocation);
	bool TryGetCurrentTarget(AIndicatingTarget*& OutTarget);

	UFUNCTION(BlueprintCallable, Category = "TargetIndicatorManager")
	void RegisterNewTarget(AIndicatingTarget* NewTarget);

#pragma endregion


};
