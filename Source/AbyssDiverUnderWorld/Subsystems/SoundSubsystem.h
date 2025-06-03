#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DataRow/SoundDataRow/BGMDataRow.h"
#include "DataRow/SoundDataRow/MonsterSFXDataRow.h"
#include "DataRow/SoundDataRow/SFXDataRow.h"
#include "DataRow/SoundDataRow/UISFXDataRow.h"
#include "DataRow/SoundDataRow/AmbientDataRow.h"

#include "Components/AudioComponent.h"

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

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 Play2D(const ESFX& SFXType, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 Play2D(const ESFX_Monster& SFXType, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 Play2D(const ESFX_UI& SFXType, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 PlayBGM(const ESFX_BGM& SFXType, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 PlayAmbient(const ESFX_Ambient& SFXType, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 PlayAt(const ESFX& SFXType, const FVector& Position, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 PlayAt(const ESFX_Monster& SFXType, const FVector& Position, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 PlayAt(const ESFX_UI& SFXType, const FVector& Position, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 PlayAttach(const ESFX& SFXType, USceneComponent* AttachComp, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 PlayAttach(const ESFX_Monster& SFXType, USceneComponent* AttachComp, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 PlayAttach(const ESFX_UI& SFXType, USceneComponent* AttachComp, const float& Volume = 1.0f, const bool& bShouldUseFadeIn = false, const float& FadeInDuration = 1.0f, const EAudioFaderCurve& FadeInCurve = (EAudioFaderCurve)0U);

	void StopAllBGM();
	void StopAllSFX();
	void StopAllAmbient();

	void StopAudio(const int32& AudioId, const bool& bShouldUseFadeOut = false, const float& FadeOutDuration = 2.0f, const float& FadeOutGoalVolume = 0.0f, const EAudioFaderCurve& FadeOutCurve = (EAudioFaderCurve)0U);
	bool IsPlaying(const int32& AudioId);
	float GetAudioTotalDuration(const int32& AudioId) const;

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

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 Play2DInternal(USoundBase* SoundAsset, const bool& bIsBGM, const bool& bIsAmbient, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 Play3DInternal(USoundBase* SoundAsset, const FVector& Position, USceneComponent* AttachComp, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve);

	// 플레이하는 오디오 아이디 반환, 실패하면 INDEX_NONE 반환
	int32 PlayAudio(UAudioComponent* Audio, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve);

	UAudioComponent* GetNewAudio();

	void CreateAudioComponent();

	// ID 만드는데 실패하면 INDEX_NONE 반환
	int32 CreateNewId();

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

	UPROPERTY()
	TMap<int32, TObjectPtr<UAudioComponent>> AudioComponentWithIdMap;

	UPROPERTY()
	TMap<TObjectPtr<UAudioComponent>, int32> AudioIdWithComponentMap;

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
