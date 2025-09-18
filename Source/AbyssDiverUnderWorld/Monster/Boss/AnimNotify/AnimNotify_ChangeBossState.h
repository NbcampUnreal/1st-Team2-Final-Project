#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ChangeBossState.generated.h"

//enum class EBossState : uint8;
enum class EMonsterState : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotify_ChangeBossState : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
									const FAnimNotifyEventReference& EventReference) override;

protected:

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//EBossState BossState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMonsterState MonsterState;
};
