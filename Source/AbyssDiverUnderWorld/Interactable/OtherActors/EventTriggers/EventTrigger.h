#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interface/Triggerable.h"

#include "EventTrigger.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AEventTrigger : public AActor
{
	GENERATED_BODY()
	
public:	

	AEventTrigger();

protected:

	virtual void PostInitializeComponents() override;

private:

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TriggerSettings")
	TObjectPtr<class UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, Category = "TriggerSettings")
	TObjectPtr<class UBillboardComponent> BillboardSprite;

	UPROPERTY(EditInstanceOnly, Category = "TriggerSettings", meta = (MustImplement = "Triggerable"));
	TArray<TObjectPtr<AActor>> TriggerableActors;
};
