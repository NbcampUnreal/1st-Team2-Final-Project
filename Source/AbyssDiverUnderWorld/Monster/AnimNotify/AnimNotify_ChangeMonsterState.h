#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"

#include "AnimNotify_ChangeMonsterState.generated.h"

enum class EMonsterState : uint8;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotify_ChangeMonsterState : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMonsterState MonsterState;
};
