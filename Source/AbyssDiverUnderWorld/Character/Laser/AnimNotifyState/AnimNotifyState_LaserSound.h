#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_LaserSound.generated.h"

class USoundBase;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotifyState_LaserSound : public UAnimNotifyState
{
	GENERATED_BODY()
	
#pragma region Method
public:
	virtual void  NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float TotalDuration, const FAnimNotifyEventReference& EventRef) override;
	virtual void  NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, const FAnimNotifyEventReference& EventRef) override;

#pragma endregion

#pragma region Variable
public:
	UPROPERTY()
	TWeakObjectPtr<UAudioComponent> CachedAudio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Laser|Sound")
	TObjectPtr<USoundBase> LoopSoundCue;

private:


#pragma endregion

#pragma region Getter, Setter

#pragma endregion
};
