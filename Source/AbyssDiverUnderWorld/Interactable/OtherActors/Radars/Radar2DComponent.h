#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Radar2DComponent.generated.h"

class URadarReturn2DComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API URadar2DComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	URadar2DComponent();

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Methods

public:

	void RegisterReturnComponent(AActor* RegisteringActor);
	void RegisterReturnComponent(URadarReturn2DComponent* RegisteringReturn);

	void UnregisterReturnComponent(AActor* RegisteredActor);
	void UnregisterReturnComponent(URadarReturn2DComponent* RegisteredReturn);

protected:

	void RegisterCurrentReturns();

	bool HasReturnAlready(URadarReturn2DComponent* RadarReturn);
	bool IsReturnInRadius(URadarReturn2DComponent* RadarReturn);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(EditAnywhere, Category = "RadarSettings")
	float RadarDetectRadius = 4000.0f;

	UPROPERTY()
	TSet<TObjectPtr<URadarReturn2DComponent>> CachedReturns;

	UPROPERTY()
	TSet<TObjectPtr<URadarReturn2DComponent>> CachedReturnsInDetectRadius;

#pragma endregion

#pragma region Getters / Setters

public:

	const TSet<TObjectPtr<URadarReturn2DComponent>>& GetAllReturnsInDetectRadius() const;
	float GetRadarDetectRadius() const;

#pragma endregion

		
};
