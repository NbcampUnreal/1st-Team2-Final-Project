#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ADBasketballHoop.generated.h"

class USoundSubsystem;
class AADBasketball;
class UBoxComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADBasketballHoop : public AActor
{
	GENERATED_BODY()
	
public:	
	AADBasketballHoop();

protected:
	virtual void BeginPlay() override;

#pragma region Method
protected:
	UFUNCTION()
	void OnGoalZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void M_OnScore(AADBasketball* Basketball);
	void M_OnScore_Implementation(AADBasketball* Basketball);

	void PlayScoreSound();

#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HoopMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> GoalZone;

	/** 득점 시 요구되는 최소 하강 속도 (음수) */
	UPROPERTY(EditAnywhere, Category = "Basketball|Goal")
	float MinimumDownwardVelocity = -100.0f;

	/** 득점 후 다시 득점 가능할 때까지의 쿨다운 */
	UPROPERTY(EditAnywhere, Category = "Basketball|Goal")
	float ScoreCooldown = 2.0f;

	float LastScoreTime = 0.0f;
	
	/** SoundSubsystem 캐시 */
	UPROPERTY()
	TWeakObjectPtr<USoundSubsystem> SoundSubsystemWeakPtr;

#pragma endregion

#pragma region Getter, Setter
public:

	bool CanScore() const;
	USoundSubsystem* GetSoundSubsystem();
#pragma endregion
};
