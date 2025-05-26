#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DataRow/SoundDataRow/BGMDataRow.h"
#include "DataRow/SoundDataRow/MonsterSFXDataRow.h"
#include "DataRow/SoundDataRow/SFXDataRow.h"
#include "DataRow/SoundDataRow/UISFXDataRow.h"

#include "SoundSubsystem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class ABYSSDIVERUNDERWORLD_API USoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region Methods

public:

	void Play2D(ESFX SFXType, float Volume = 1.0f);
	void Play2D(ESFX_Monster SFXType, float Volume = 1.0f);
	void Play2D(ESFX_UI SFXType, float Volume = 1.0f);
	void PlayBGM(ESFX_BGM SFXType, float Volume = 1.0f);

	void PlayAt(ESFX SFXType, const FVector& Position, float Volume = 1.0f);
	void PlayAt(ESFX_Monster SFXType, const FVector& Position, float Volume = 1.0f);
	void PlayAt(ESFX_UI SFXType, const FVector& Position, float Volume = 1.0f);

	void PlayAttach(ESFX SFXType, USceneComponent* AttachComp, float Volume = 1.0f);
	void PlayAttach(ESFX_Monster SFXType, USceneComponent* AttachComp, float Volume = 1.0f);
	void PlayAttach(ESFX_UI SFXType, USceneComponent* AttachComp, float Volume = 1.0f);

	void StopAllBGM();
	void StopAllSFX();

	// 0~1의 값
	void ChangeMasterVolume(float NewMasterVolume);

	// 0~1의 값
	void ChangeBGMVolume(float NewBGMVolume);

	// 0~1의 값
	void ChangeSFXVolume(float NewSFXVolume);

private:

	void ChangeVolumeInternal();

	void OnAudioFinished(UAudioComponent* FinishedAudio);

	void PlayInternal(USoundBase* SoundAsset, bool bIsBGM = false, bool bIs2D = true, const FVector& Position = FVector::ZeroVector, USceneComponent* AttachComp = nullptr, float Volume = 1.0f);

#pragma endregion

#pragma region Variables

private:

	TArray<FSFXDataRow*> SFXData;

	TArray<FMonsterSFXDataRow*> SFXMonsterData;

	TArray<FUISFXDataRow*> SFXUIData;

	TArray<FBGMDataRow*> SFXBGMData;

	// 원래 볼륨을 Value로 저장
	UPROPERTY()
	TMap<TObjectPtr<UAudioComponent>, float> ActivatedSFXComponents;

	// 원래 볼륨을 Value로 저장
	UPROPERTY()
	TMap<TObjectPtr<UAudioComponent>, float> ActivatedBGMComponents;

	TQueue<TObjectPtr<UAudioComponent>> DeactivatedComponents;

	int32 SFXDataCount = 0;
	int32 SFXMonsterDataCount = 0;
	int32 SFXUIDataCount = 0;
	int32 SFXBGMDataCount = 0;

	float MasterVolume = 1.0f;
	float BGMVolume = 1.0f;
	float SFXVolume = 1.0f;

#pragma endregion

#pragma region Getters / Setters

public:

	FORCEINLINE float GetMasterVolume() const { return MasterVolume; }
	FORCEINLINE float GetBGMVolume() const { return BGMVolume; }
	FORCEINLINE float GetSFXVolume() const { return SFXVolume; }

#pragma endregion

};
