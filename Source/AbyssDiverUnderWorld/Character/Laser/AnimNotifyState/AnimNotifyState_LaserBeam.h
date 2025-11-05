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
	float CalculateDistanceBasedScale(float Distance) const;
	float CalculateDistanceBasedIntensity(float Distance) const;
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
	FName MuzzleSocket = "LaserMuzzle";

	// Niagara 빔에서 사용자 파라미터 이름(EndPoint)
	UPROPERTY(EditAnywhere, Category = "Laser")
	FName BeamEndParam = "LaserEnd";

	// Niagara 빔에서 사용자 파라미터 이름(LoopDuration)
	UPROPERTY(EditAnywhere, Category = "Laser")
	FName DurationParam = "LoopDuration";

	// 1인칭 시점 이펙트 크기 비율
	UPROPERTY(EditAnywhere, Category = "Laser", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FirstPersonImpactScale = 0.3f;    

	/* 거리 기반 조정 파라미터들 **/ 
	// 최소 거리
	UPROPERTY(EditAnywhere, Category = "Laser|Distance", meta = (ClampMin = "0.0"))
	float MinEffectDistance = 100.0f;  
	// 최대 거리 (이 거리 이상에서는 원본 크기)
	UPROPERTY(EditAnywhere, Category = "Laser|Distance", meta = (ClampMin = "0.0"))
	float MaxEffectDistance = 1000.0f; 
	// 최소 스케일 (20%)
	UPROPERTY(EditAnywhere, Category = "Laser|Distance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinEffectScale = 0.2f;  
	// 최대 스케일 (100%)
	UPROPERTY(EditAnywhere, Category = "Laser|Distance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxEffectScale = 1.0f;  
	// 최소 강도
	UPROPERTY(EditAnywhere, Category = "Laser|Distance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinEffectIntensity = 0.1f;  
	// 최대 강도
	UPROPERTY(EditAnywhere, Category = "Laser|Distance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxEffectIntensity = 1.0f;  
	// 카메라로부터 최소 거리
	UPROPERTY(EditAnywhere, Category = "Laser|FirstPerson", meta = (ClampMin = "50.0"))
	float MinCameraOffset = 150.0f;  

	/* Niagara 파라미터 이름들 **/ 
 	UPROPERTY(EditAnywhere, Category = "Laser|Niagara")
	FName IntensityParam = "Intensity";
	UPROPERTY(EditAnywhere, Category = "Laser|Niagara")
	FName OpacityParam = "Opacity";
private:
	static TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> BeamMap1P;
	static TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> BeamMap3P;
	static TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> HitMap1P;
	static TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> HitMap3P;

	TObjectPtr<AUnderwaterCharacter> Diver;
#pragma endregion
};
