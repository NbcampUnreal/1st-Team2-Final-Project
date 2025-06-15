// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BonePose.h"
#include "RagdollTypes.h"
#include "RagdollReplicationComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABYSSDIVERUNDERWORLD_API URagdollReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URagdollReplicationComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Methode

public:

	/** 래그돌 활성화. 모든 Node에서 호출을 해야 한다.
	 * 활성화되면 Server에서 Transform을 갱신한다.
	 * Client에서는 OnRep_BoneTransforms를 통해서 Transform을 갱신한다.
	 */
	void SetRagdollEnabled(bool bEnable);
	
protected:

	/** 래그돌에서 Transform을 캡처한다.
	 * 본의 위치, 회전, 스케일, 선형 속도, 각속도를 캡처한다.
	 */
	void CaptureBoneTransforms();

	/** 래그돌 레플리케이션 함수. 래그돌이 활성화일 때만 동기화 된다. */
	UFUNCTION()
	void OnRep_BoneTransforms();

#pragma endregion

#pragma region Variable

protected:

	/** 래그돌의 Transform을 저장하는 구조체 배열 */
	UPROPERTY(ReplicatedUsing = OnRep_BoneTransforms)
	TArray<FReplicatedBoneTransform> BoneTransforms;
	
private:

	/** 래그돌에 사용할 스켈레탈 메시 컴포넌트 */
	UPROPERTY()
	TObjectPtr<class USkeletalMeshComponent> SkeletalMesh;

	/** 캡처할 본 이름 */
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> CaptureBoneNames;

	/** 래그돌 활성화 여부 */
	bool bIsRagdoll;

	/** 트랜스폼을 저장하는 간격 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Ragdoll")
	float SnapshotInterval;

	/** 가장 최근에 트랜스폼을 캡처한 시간 */
	float TimeSinceLastSnapshot;

#pragma endregion
};

