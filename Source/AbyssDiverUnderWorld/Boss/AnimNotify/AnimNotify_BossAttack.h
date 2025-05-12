#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_BossAttack.generated.h"

class UCapsuleComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotify_BossAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_BossAttack();
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CollisionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackInterval;

private:
	FTimerHandle AttackTimer;
};
