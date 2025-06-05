#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SetBlackboardKey.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotify_SetBlackboardKey : public UAnimNotify
{
	GENERATED_BODY()

private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere)
	TMap<FString, bool> BlackboardKeyValues;
	
};
