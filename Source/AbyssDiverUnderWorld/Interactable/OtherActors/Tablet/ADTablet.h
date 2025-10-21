#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "ADTablet.generated.h"

class UWidgetComponent;
class UCameraComponent;
class AUnderwaterCharacter;
class USoundSubsystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTablet : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AADTablet();

public:
	virtual void BeginPlay() override;

#pragma region Method
public:
	/** Interactable Interface Begin */
	virtual UADInteractableComponent* GetInteractableComponent() const override { return InteractableComp; }
	virtual bool IsHoldMode() const override { return false; }
	virtual FString GetInteractionDescription() const override;
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanHighlight_Implementation() const override { return !HeldBy; }
	/** Interactable Interface End */

	/** 태블릿을 들게 한다. */
	void Pickup(AUnderwaterCharacter* UnderwaterCharacter);

	/** 태블릿을 내려놓게 한다. */
	UFUNCTION(BlueprintCallable)
	void PutDown();

protected:
	
	UFUNCTION()
	void OnRep_HeldBy();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty>& OutLifetimeProps) const override;

	/** 감정 표현 시작 시 호출. 현재 Screen Widget을 숨긴다. */
	UFUNCTION()
	void OnEmoteStart();

	/** 감정 표현 종료 시 호출. 현재 Screen Widget을 다시 보이게 한다. */
	UFUNCTION()
	void OnEmoteEnd();

private:
#pragma endregion

#pragma region Variable
public:

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	/** 태블릿 메시 컴포넌트 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> TabletMesh;

	/** 태블릿 화면 위젯 컴포넌트 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UWidgetComponent> ScreenWidget;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UADInteractableComponent> InteractableComp;

private:

	/** 현재 이 태블릿을 들고 있는 캐릭터 */
	UPROPERTY(ReplicatedUsing = OnRep_HeldBy)
	TObjectPtr<AUnderwaterCharacter> HeldBy;

	/** HeldBy의 약한 참조 포인터, Guest에서 Prev HeldBy를 추적하기 위해 사용 */
	TWeakObjectPtr<AUnderwaterCharacter> HeldByWeakPtr;
	
	UPROPERTY(EditAnywhere)
	FVector HoldOffsetLocation = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere)
	FRotator HoldOffsetRotation = FRotator(0.f, 0.f, 0.f);

	UPROPERTY()
	TWeakObjectPtr<USoundSubsystem> SoundSubsystemWeakPtr;
	
#pragma endregion

#pragma region Getter, Setteer

	/** 태블릿이 현재 들려져 있는지 여부를 반환 */
	bool IsHeld() const { return HeldBy != nullptr; }
	
private:
	
	USoundSubsystem* GetSoundSubsystem();
	
#pragma endregion
};
