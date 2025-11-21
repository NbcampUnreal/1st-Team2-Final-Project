#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/IADInteractable.h"
#include "DataRow/SoundDataRow/BGMDataRow.h"
#include "ADRadio.generated.h"

class USoundSubsystem;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADRadio : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	
	AADRadio();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanHighlight_Implementation() const override { return true; }
	virtual UADInteractableComponent* GetInteractableComponent() const override { return InteractableComp; }
	virtual bool IsHoldMode() const override { return false; }
	virtual FString GetInteractionDescription() const override;

protected:
	UFUNCTION()
	void OnRep_IsOn();

	void TurnOn();
	void TurnOff();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
#pragma endregion

#pragma region Variable
public:
	/** Radio 소리 관련 변수 */
	UPROPERTY(EditAnywhere, Category = "Radio|Sound", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float RadioVolume = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Radio|Sound")
	bool bUseFade = true;
	UPROPERTY(EditAnywhere, Category = "Radio|Sound", meta = (EditCondition = "bUseFade", ClampMin = "0.0"))
	float FadeInTime = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Radio|Sound", meta = (EditCondition = "bUseFade", ClampMin = "0.0"))
	float FadeOutTime = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Radio|Sound")
	ESFX_BGM RadioBGM = ESFX_BGM::CampRadio;

	// 상태별 상호작용 문구
	UPROPERTY(EditAnywhere, Category = "Radio|UI")
	FText DescWhenOn = NSLOCTEXT("Radio", "DescWhenOn", "Turn Off Radio");
	UPROPERTY(EditAnywhere, Category = "Radio|UI")
	FText DescWhenOff = NSLOCTEXT("Radio", "DescWhenOff", "Turn On Radio");

protected:


private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> RadioMesh;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UADInteractableComponent> InteractableComp;
	UPROPERTY()
	TWeakObjectPtr<USoundSubsystem> SoundSubsystem;
	UPROPERTY(ReplicatedUsing = OnRep_IsOn)
	uint8 bIsOn : 1 = false;
	UPROPERTY()
	int32 CurrentBGMId = INDEX_NONE;
#pragma endregion

#pragma region Getter, Setteer
public:
	USoundSubsystem* GetSoundSubsystem();
#pragma endregion

};
