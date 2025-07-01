#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_LaserBeam.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class AUnderwaterCharacter;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAnimNotifyState_LaserBeam : public UAnimNotifyState
{
	GENERATED_BODY()
	
	
#pragma region Method

public:
	virtual void  NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float TotalDuration, const FAnimNotifyEventReference& EventRef) override;
	virtual void  NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, const FAnimNotifyEventReference& EventRef) override;
	virtual void  NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float FrameDeltaTime, const FAnimNotifyEventReference& EventRef) override;

private:
	static bool IsVisibleMesh(const USkeletalMeshComponent* MeshComp);
	static bool IsFirstPersonMesh(const USkeletalMeshComponent* MeshComp);
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditAnywhere, Category = "Laser")
	TObjectPtr<UNiagaraSystem> BeamFX = nullptr;               // �߻缱

	UPROPERTY(EditAnywhere, Category = "Laser")
	TObjectPtr<UNiagaraSystem> ImpactFX = nullptr;             // �ǰ� ����

	// ��� ����(�Ÿ�) 
	UPROPERTY(EditAnywhere, Category = "Laser", meta = (ClampMin = "10.0"))
	float MaxDistance = 3000.0f;

	// �ѱ� ���� �̸�
	UPROPERTY(EditAnywhere, Category = "Laser")
	FName MuzzleSocket = "LaserMuzzle";

	// Niagara ������ ����� �Ķ���� �̸�(EndPoint)
	UPROPERTY(EditAnywhere, Category = "Laser")
	FName BeamEndParam = "LaserEnd";

	// Niagara ������ ����� �Ķ���� �̸�(LoopDuration)
	UPROPERTY(EditAnywhere, Category = "Laser")
	FName DurationParam = "LoopDuration";

private:
	static TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> BeamMap1P;
	static TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> BeamMap3P;
	static TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> HitMap1P;
	static TMap<TObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> HitMap3P;

	TObjectPtr<AUnderwaterCharacter> Diver;
#pragma endregion
};
