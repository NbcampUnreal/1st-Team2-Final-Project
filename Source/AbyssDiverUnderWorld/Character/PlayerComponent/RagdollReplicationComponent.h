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

	/** 래그돌 레플리케이션 함수. 래그돌이 활성화일 때만 동기화 된다.
	 * Server의 Bone Transform에 따라 Client의 Boen Transform을 갱신한다.
	 */
	UFUNCTION()
	void OnRep_BoneTransforms();

	/** 캡슐 컴포넌트의 위치와 회전을 래그돌의 특정 본에 맞추어 업데이트한다.
	 * 캡슐 컴포넌트의 위치를 갱신함으로써 카메라가 래그돌에 동기화 된다.
	 */
	void UpdateCapsuleTransform();

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

	// @ToDo: 불필요한 본을 제거해서 네트워크 최적화
	
	/** 캡처할 본 이름. Bone Instances를 복제하기 때문에 PA에서 정의된 Bone의 이름을 지정해야 한다.
	 * Root는 현재 이유는 알 수 없으나 포함되어 있어야 제대로 동작한다.
	 */
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> CaptureBoneNames;

	/** 래그돌 활성화 여부 */
	uint8 bIsRagdoll : 1;

	/** 트랜스폼을 저장하는 간격 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Ragdoll")
	float SnapshotInterval;

	/** 가장 최근에 트랜스폼을 캡처한 시간 */
	float TimeSinceLastSnapshot;

	/** 캡슐 컴포넌트의 위치와 회전을 따라갈 본의 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Ragdoll")
	FName CapsuleTargetBoneName;

	/** 캡슐과 본 사이의 오프셋 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Ragdoll")
	FVector CapsuleBoneOffset;

	/** 캡슐 컴포넌트 레퍼런스 캐싱 */
	UPROPERTY()
	TObjectPtr<class UCapsuleComponent> CapsuleComponent;

#pragma endregion
};

