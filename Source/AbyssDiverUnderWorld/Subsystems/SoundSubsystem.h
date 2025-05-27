#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DataRow/SoundDataRow/BGMDataRow.h"
#include "DataRow/SoundDataRow/MonsterSFXDataRow.h"
#include "DataRow/SoundDataRow/SFXDataRow.h"
#include "DataRow/SoundDataRow/UISFXDataRow.h"
#include "DataRow/SoundDataRow/AmbientDataRow.h"

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

	void Init(const int32 InitialPoolCount);

	void Play2D(const ESFX& SFXType, const float& Volume = 1.0f);
	void Play2D(const ESFX_Monster& SFXType, const float& Volume = 1.0f);
	void Play2D(const ESFX_UI& SFXType, const float& Volume = 1.0f);
	void PlayBGM(const ESFX_BGM& SFXType, const float& Volume = 1.0f);
	void PlayAmbient(const ESFX_Ambient& SFXType, const float& Volume = 1.0f);

	void PlayAt(const ESFX& SFXType, const FVector& Position, const float& Volume = 1.0f);
	void PlayAt(const ESFX_Monster& SFXType, const FVector& Position, const float& Volume = 1.0f);
	void PlayAt(const ESFX_UI& SFXType, const FVector& Position, const float& Volume = 1.0f);

	void PlayAttach(const ESFX& SFXType, USceneComponent* AttachComp, const float& Volume = 1.0f);
	void PlayAttach(const ESFX_Monster& SFXType, USceneComponent* AttachComp, const float& Volume = 1.0f);
	void PlayAttach(const ESFX_UI& SFXType, USceneComponent* AttachComp, const float& Volume = 1.0f);

	void StopAllBGM();
	void StopAllSFX();
	void StopAllAmbient();

	// 0~1의 값
	void ChangeMasterVolume(const float& NewMasterVolume);

	// 0~1의 값
	void ChangeBGMVolume(const float& NewBGMVolume);

	// 0~1의 값
	void ChangeSFXVolume(const float& NewSFXVolume);

	// 0~1의 값
	void ChangeAmbientVolume(const float& NewAmbientVolume);

private:

	void ChangeMasterVolumeInternal();
	void ChangeBGMVolumeInternal();
	void ChangeSFXVolumeInternal();
	void ChangeAmbientVolumeInternal();

	void OnAudioFinished(UAudioComponent* FinishedAudio);

	void Play2DInternal(USoundBase* SoundAsset, const bool& bIsBGM, const bool& bIsAmbient, const float& Volume = 1.0f);
	void Play3DInternal(USoundBase* SoundAsset, const FVector& Position, USceneComponent* AttachComp = nullptr, const float& Volume = 1.0f);

	void PlayAudio(UAudioComponent* Audio);

	UAudioComponent* GetNewAudio();

	void Create();

#pragma endregion

#pragma region Variables

private:

	TArray<FSFXDataRow*> SFXData;

	TArray<FMonsterSFXDataRow*> SFXMonsterData;

	TArray<FUISFXDataRow*> SFXUIData;

	TArray<FBGMDataRow*> SFXBGMData;

	TArray<FAmbientDataRow*> AmbientData;

	// 원래 볼륨을 Value로 저장
	UPROPERTY()
	TMap<TObjectPtr<UAudioComponent>, float> ActivatedSFXComponents;

	// 원래 볼륨을 Value로 저장
	UPROPERTY()
	TMap<TObjectPtr<UAudioComponent>, float> ActivatedBGMComponents;

	// 원래 볼륨을 Value로 저장
	UPROPERTY()
	TMap<TObjectPtr<UAudioComponent>, float> ActivatedAmbientComponents;

	TQueue<TObjectPtr<UAudioComponent>> DeactivatedComponents;

	int32 SFXDataCount = 0;
	int32 SFXMonsterDataCount = 0;
	int32 SFXUIDataCount = 0;
	int32 SFXBGMDataCount = 0;
	int32 AmbientDataCount = 0;

	float MasterVolume = 0.5f;
	float BGMVolume = 0.5f;
	float SFXVolume = 0.5f;
	float AmbientVolume = 0.5f;

#pragma endregion

#pragma region Getters / Setters

public:

	FORCEINLINE float GetMasterVolume() const { return MasterVolume; }
	FORCEINLINE float GetBGMVolume() const { return BGMVolume; }
	FORCEINLINE float GetSFXVolume() const { return SFXVolume; }
	FORCEINLINE float GetAmbientVolume() const { return AmbientVolume; }

#pragma endregion

};
