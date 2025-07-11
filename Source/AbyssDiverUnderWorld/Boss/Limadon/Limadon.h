#pragma once

#include "CoreMinimal.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Limadon.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ALimadon : public ABoss
{
	GENERATED_BODY()

public:
	ALimadon();

public:
	virtual void BeginPlay() override;
	
public:
	virtual void Attack() override;
	virtual void OnDeath() override;
	
	void BiteVariableInitialize();

	UFUNCTION(BlueprintImplementableEvent)
	void SetEmissiveUp();

	UFUNCTION(BlueprintImplementableEvent)
	void SetEmissiveDown();
	
protected:
	UFUNCTION(BlueprintCallable)
	void Spit();
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float StopCaptureHealthCriteria;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> HideAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> SpitAnimation;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> BiteAttackCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Mesh")
	TObjectPtr<USkeletalMeshComponent> LeftSphereMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Mesh")
	TObjectPtr<USkeletalMeshComponent> RightSphereMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Hide")
	float HideTime;

private:
	uint8 bIsInvestigate : 1;
	FTimerHandle InvestigateTimerHandle;

public:
	FORCEINLINE bool GetIsInvestigate() const { return bIsInvestigate; }
	FORCEINLINE void SetInvestigateMode()
	{
		bIsInvestigate = true;LOG(TEXT("Begin"));
	}

};
