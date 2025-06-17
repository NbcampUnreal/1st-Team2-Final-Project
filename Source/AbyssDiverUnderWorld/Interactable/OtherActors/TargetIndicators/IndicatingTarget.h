#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "IndicatingTarget.generated.h"

class UBoxComponent;
class UBillboardComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnIndicatingTargetBeginOverlapDelegate, int32 /*TargetOrder*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSwitchActorDestroyedDelegate, int32 /*TargetOrder*/);


UCLASS()
class ABYSSDIVERUNDERWORLD_API AIndicatingTarget : public AActor
{
	GENERATED_BODY()
	
public:	

	AIndicatingTarget();

public:

	virtual void BeginPlay() override;

#pragma region Methods

public:

	bool IsActivateConditionMet();

	FOnIndicatingTargetBeginOverlapDelegate OnIndicatingTargetBeginOverlapDelegate;
	FOnSwitchActorDestroyedDelegate OnSwitchActorDestroyedDelegate;

private:

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSwitchActorDestroyed(AActor* DestroyedActor);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(VisibleAnywhere, Category = "IndicatingTarget")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, Category = "IndicatingTarget")
	TObjectPtr<UBillboardComponent> BillboardSprite;

	UPROPERTY(EditInstanceOnly, Category = "IndicatingTargetSettings")
	int32 TargetOrder;

	UPROPERTY(EditInstanceOnly, Category = "IndicatingTargetSettings")
	TObjectPtr<UTexture2D> TargetIcon;

	// 이 액터가 사라지면 활성화 조건 만족
	UPROPERTY(EditInstanceOnly, Category = "IndicatingTargetSettings")
	TObjectPtr<AActor> SwitchActor;

#pragma endregion

#pragma region Getters / Setters

public:

	int32 GetTargetOrder() const;
	UTexture2D* GetTargetIcon() const;

#pragma endregion


};
