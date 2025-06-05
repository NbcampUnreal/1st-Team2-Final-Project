#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_LaserBeam.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotifyState_LaserBeam : public UAnimNotifyState
{
	GENERATED_BODY()
	
	
#pragma region Method

public:
	virtual void  NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float TotalDuration, const FAnimNotifyEventReference& EventRef) override;
	virtual void  NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, const FAnimNotifyEventReference& EventRef) override;
	virtual void  NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float FrameDeltaTime, const FAnimNotifyEventReference& EventRef) override;

#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditAnywhere, Category = "Laser")
	TObjectPtr<UNiagaraSystem> BeamFX = nullptr;               // 발사선

	UPROPERTY(EditAnywhere, Category = "Laser")
	TObjectPtr<UNiagaraSystem> ImpactFX = nullptr;             // 피격 지점

	// 쏘는 길이(거리) 
	UPROPERTY(EditAnywhere, Category = "Laser", meta = (ClampMin = "10.0"))
	float MaxDistance = 3000.0f;

	// 총구 소켓 이름
	UPROPERTY(EditAnywhere, Category = "Laser")
	FName MuzzleSocket = "Muzzle";

	// Niagara 빔에서 사용자 파라미터 이름(EndPoint)
	UPROPERTY(EditAnywhere, Category = "Laser")
	FName BeamEndParam = "LaserEnd";

	// Niagara 빔에서 사용자 파라미터 이름(LoopDuration)
	UPROPERTY(EditAnywhere, Category = "Laser")
	FName DurationParam = "LoopDuration";

private:
	TWeakObjectPtr<UNiagaraComponent> CachedSpawnedBeam;
	TWeakObjectPtr<UNiagaraComponent> CachedHitEffect;
#pragma endregion
};
