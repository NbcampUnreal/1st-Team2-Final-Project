// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CableBindingActor.generated.h"

class AUnderwaterCharacter;
/*
 * 2개의 Actor를 연결하는 Actor
 * - Source Actor: Cable이 연결되어서 끌고 가는 Actor
 * - Target Actor: Cable이 연결되어서 끌려가는 Actor
 * Target Actor는 물리 시뮬레이션이 활성화되어 있어야 한다.
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API ACableBindingActor : public AActor
{
	GENERATED_BODY()

public:
	ACableBindingActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

#pragma region Method

public:

	/** Source Actor와 Target Actor를 연결한다.
	 * Source Actor는 Cable의 시작점이 되고, Target Actor는 Cable의 끝점이 된다.
	 * Target Actor는 물리 시뮬레이션이 활성화되어 있어야 한다.
	 */
	UFUNCTION(BlueprintCallable)
	void ConnectActors(AUnderwaterCharacter* NewSourceActor, AUnderwaterCharacter* NewTargetActor);

	/** Source Actor와 Target Actor를 분리한다.
	 * Cable을 제거하고, Physics Constraint를 비활성화한다.
	 */
	UFUNCTION(BlueprintCallable)
	void DisconnectActors();

	/** CableComponent에 World Offset을 적용한다.
	 * Particle에 직접적으로 접근할 수 없으므로 ApplyWorldOffset을 통해서 간접적으로 OldPosition을 갱신한다.
	 */
	UFUNCTION(BlueprintCallable)
	void ApplyWorldOffsetToCable(FVector Offset);

protected:
	/** 현재 Cable이 유효한지 확인한다.
	 * Source Actor와 Target Actor가 모두 유효한지 확인한다.
	 */
	bool IsConnectionValid() const;
	
#pragma endregion

#pragma region Variable

protected:

	/** Cable Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UCableComponent> CableComponent;

	/** 물리 제약을 위한 Physics Constraint Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UPhysicsConstraintComponent> PhysicsConstraint;

	/** Cable이 연결되는 Source Actor. Source Actor는 끌고가는 Actor이다.(Binder Actor) */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AUnderwaterCharacter> SourceCharacter;

	/** Cable이 연결되는 Target Actor. Target Actor는 끌려가는 Actor이다.(Bound Actor) */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AUnderwaterCharacter> TargetCharacter;

	/** Cable의 길이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CableLength;

	/** Constraint의 강성도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConstraintStiffness;

	/** Constraint의 감쇠력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConstraintDamping;
	
#pragma endregion

#pragma region Getter Setter

public:

	/** 현재 연결 상태를 반환 */
	UFUNCTION(BlueprintCallable)
	bool IsConnected() const;
	
#pragma endregion
};
