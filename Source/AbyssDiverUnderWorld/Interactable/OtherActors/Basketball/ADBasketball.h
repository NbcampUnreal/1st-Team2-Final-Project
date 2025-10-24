#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "ADBasketball.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class AUnderwaterCharacter;
class USoundSubsystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADBasketball : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	AADBasketball();

public:
	virtual void BeginPlay() override;

#pragma region Method
public:
	virtual UADInteractableComponent* GetInteractableComponent() const override { return InteractableComp; }
	virtual bool IsHoldMode() const override { return false; }
	virtual FString GetInteractionDescription() const override;
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanHighlight_Implementation() const override { return !HeldBy; }

	UFUNCTION(BlueprintCallable)
	void Pickup(AUnderwaterCharacter* Diver);
	
	UFUNCTION(BlueprintCallable)
	void Throw();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OhterComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UFUNCTION()
	void OnRep_HeldBy();
	UFUNCTION()
	void OnRep_bIsThrown();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 던진 후 일정 시간 후에 다시 줍기 가능하게 만들기 */
	void EnablePickupAfterThrow();
	
	void KnockbackPlayer(AUnderwaterCharacter* TargetPlayer, const FVector& ImpactPoint);

	/** 농구 골대 득점 처리 -> 나중에 필요한지 생각해보기 */
	void ScoreBasket();

	void PlayBounceSound(float ImpactStrength);

private:
	UPROPERTY(EditAnywhere, Category = "Basketball|Attachment")
	FName BasketballSocketName = TEXT("Basketball");
	FVector CalculateThrowVelocity() const;
	

#pragma endregion

#pragma region Variable
protected:
	/** 에셋에 따라 스켈레탈 메시로 바뀔 수도 있음 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> BallMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollisionSphere;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	UPROPERTY()
	TObjectPtr<UADInteractableComponent> InteractableComp;
	
	UPROPERTY(ReplicatedUsing = OnRep_HeldBy)
	TObjectPtr<AUnderwaterCharacter> HeldBy;
	
	TWeakObjectPtr<AUnderwaterCharacter> CachedHeldBy;

	UPROPERTY(ReplicatedUsing = OnRep_bIsThrown)
	uint8 bIsThrown : 1 = false;

	UPROPERTY(EditAnywhere, Category = "Baseketball")
	float PickupCooldown = 0.5f;
	
	/** 던지기 힘 */
	UPROPERTY(EditAnywhere, Category = "Basketball|Throw")
	float ThrowForce = 1500.0f;
	/** 위쪽 던지기 추가 힘 (아크를 만들기 위한 것) */
	UPROPERTY(EditAnywhere, Category = "Basketball|Throw")
	float ThrowUpwardForce = 500.0f;
	/** 플레이어 넉백 힘 */
	UPROPERTY(EditAnywhere, Category = "Basketball|Knockback")
	float KnockbackForce = 800.0f;
	/** 넉백을 일으킬 최소 속도 */
	UPROPERTY(EditAnywhere, Category = "Basketball|Knockback")
	float MinVelocityForKnockback = 300.0f;
	UPROPERTY(EditAnywhere, Category = "Basketball|Knockback")
	float KnockbackZDirection = 0.3f;
	UPROPERTY()
	TWeakObjectPtr<USoundSubsystem> SoundSubsystemWeakPtr;

	FTimerHandle PickupEnableTimerHandle;

	float LastBounceSoundTime = 0.0f;

#pragma endregion

#pragma region Getter, Setter
public:
	UFUNCTION(BlueprintCallable, Category = "Basketball")
	bool IsHeld() const { return HeldBy != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Basketball")
	bool IsThrown() const { return bIsThrown; }

private:
	USoundSubsystem* GetSoundSubsystem();

#pragma endregion
};
