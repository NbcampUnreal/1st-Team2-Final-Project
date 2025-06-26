#include "Subsystems/SoundSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "Framework/ADInGameState.h"

void USoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &USoundSubsystem::OnWorldTearDown);

	UADGameInstance* GI = CastChecked<UADGameInstance>(GetGameInstance());
	GI->SFXDataTable->GetAllRows<FSFXDataRow>(TEXT("Getting SFXs.."), SFXData);
	SFXDataCount = SFXData.Num();

	Algo::Sort(SFXData, [](const FSFXDataRow* A, const FSFXDataRow* B)
		{
			return A->SoundType < B->SoundType;
		});

	for (int32 i = 0; i < SFXDataCount; ++i)
	{
		if (ensureMsgf(i == (int32)SFXData[i]->SoundType, TEXT("DT_SFX에 %d 번에 해당하는 사운드 데이터가 없습니다. 반드시 0번부터 빈 Enum 없이 채워주세요"), i) == false)
		{
			break;
		}
	}

	LOGV(Log, TEXT("SFXData is Initialized, Count : %d"), SFXDataCount);

	GI->BGMDataTable->GetAllRows<FBGMDataRow>(TEXT("Getting BGMs.."), SFXBGMData);
	SFXBGMDataCount = SFXBGMData.Num();

	Algo::Sort(SFXBGMData, [](const FBGMDataRow* A, const FBGMDataRow* B)
		{
			return A->SoundType < B->SoundType;
		});

	for (int32 i = 0; i < SFXBGMDataCount; ++i)
	{
		if (ensureMsgf(i == (int32)SFXBGMData[i]->SoundType, TEXT("DT_BGM에 %d 번에 해당하는 사운드 데이터가 없습니다. 반드시 0번부터 빈 Enum 없이 채워주세요"), i) == false)
		{
			break;
		}
	}

	LOGV(Log, TEXT("SFXBGMData is Initialized, Count : %d"), SFXBGMDataCount);

	GI->UISFXDataTable->GetAllRows<FUISFXDataRow>(TEXT("Getting UI SFXs.."), SFXUIData);
	SFXUIDataCount = SFXUIData.Num();

	Algo::Sort(SFXUIData, [](const FUISFXDataRow* A, const FUISFXDataRow* B)
		{
			return A->SoundType < B->SoundType;
		});

	for (int32 i = 0; i < SFXUIDataCount; ++i)
	{
		if (ensureMsgf(i == (int32)SFXUIData[i]->SoundType, TEXT("DT_UISFX에 %d 번에 해당하는 사운드 데이터가 없습니다. 반드시 0번부터 빈 Enum 없이 채워주세요"), i) == false)
		{
			break;
		}
	}

	LOGV(Log, TEXT("SFXUIData is Initialized, Count : %d"), SFXUIDataCount);

	GI->MonsterSFXDataTable->GetAllRows<FMonsterSFXDataRow>(TEXT("Getting Monster SFXs.."), SFXMonsterData);
	SFXMonsterDataCount = SFXMonsterData.Num();

	Algo::Sort(SFXMonsterData, [](const FMonsterSFXDataRow* A, const FMonsterSFXDataRow* B)
		{
			return A->SoundType < B->SoundType;
		});

	for (int32 i = 0; i < SFXMonsterDataCount; ++i)
	{
		if (ensureMsgf(i == (int32)SFXMonsterData[i]->SoundType, TEXT("DT_MonsterSFX에 %d 번에 해당하는 사운드 데이터가 없습니다. 반드시 0번부터 빈 Enum 없이 채워주세요"), i) == false)
		{
			break;
		}
	}
	
	LOGV(Log, TEXT("SFXMonsterData is Initialized, Count : %d"), SFXMonsterDataCount);

	GI->AmbientDataTable->GetAllRows<FAmbientDataRow>(TEXT("Getting Monster SFXs.."), AmbientData);
	AmbientDataCount = AmbientData.Num();

	Algo::Sort(AmbientData, [](const FAmbientDataRow* A, const FAmbientDataRow* B)
		{
			return A->SoundType < B->SoundType;
		});

	for (int32 i = 0; i < AmbientDataCount; ++i)
	{
		if (ensureMsgf(i == (int32)AmbientData[i]->SoundType, TEXT("DT_AmbientSFX에 %d 번에 해당하는 사운드 데이터가 없습니다. 반드시 0번부터 빈 Enum 없이 채워주세요"), i) == false)
		{
			break;
		}
	}

	LOGV(Log, TEXT("AmbientData is Initialized, Count : %d"), AmbientDataCount);
}

void USoundSubsystem::Init(const int32 InitialPoolCount)
{
	ActivatedAmbientComponents.Reset();
	ActivatedAmbientComponents.Reserve(InitialPoolCount);

	ActivatedBGMComponents.Reset();
	ActivatedBGMComponents.Reserve(InitialPoolCount);

	ActivatedSFXComponents.Reset();
	ActivatedSFXComponents.Reserve(InitialPoolCount);

	AudioComponentWithIdMap.Reset();
	AudioComponentWithIdMap.Reserve(InitialPoolCount);

	AudioIdWithComponentMap.Reset();
	AudioIdWithComponentMap.Reserve(InitialPoolCount);

	DeactivatedComponents.Empty();

	for (int32 i = 0; i < InitialPoolCount; ++i)
	{
		CreateAudioComponent();
	}
}

int32 USoundSubsystem::Play2D(const ESFX& SFXType, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play2DInternal(SFXData[int32(SFXType)]->Sound, false, false, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::Play2D(const ESFX_Monster& SFXType, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play2DInternal(SFXMonsterData[int32(SFXType)]->Sound, false, false, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::Play2D(const ESFX_UI& SFXType, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play2DInternal(SFXUIData[int32(SFXType)]->Sound, false, false, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::PlayBGM(const ESFX_BGM& SFXType, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play2DInternal(SFXBGMData[int32(SFXType)]->Sound, true, false, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::PlayAmbient(const ESFX_Ambient& SFXType, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play2DInternal(AmbientData[int32(SFXType)]->Sound, false, true, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::PlayAt(const ESFX& SFXType, const FVector& Position, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play3DInternal(SFXData[int32(SFXType)]->Sound, Position, nullptr, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::PlayAt(const ESFX_Monster& SFXType, const FVector& Position, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play3DInternal(SFXMonsterData[int32(SFXType)]->Sound, Position, nullptr, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::PlayAt(const ESFX_UI& SFXType, const FVector& Position, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play3DInternal(SFXUIData[int32(SFXType)]->Sound, Position, nullptr, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::PlayAttach(const ESFX& SFXType, USceneComponent* AttachComp, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play3DInternal(SFXData[int32(SFXType)]->Sound, FVector::ZeroVector, AttachComp, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::PlayAttach(const ESFX_Monster& SFXType, USceneComponent* AttachComp, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play3DInternal(SFXMonsterData[int32(SFXType)]->Sound, FVector::ZeroVector, AttachComp, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::PlayAttach(const ESFX_UI& SFXType, USceneComponent* AttachComp, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	return Play3DInternal(SFXUIData[int32(SFXType)]->Sound, FVector::ZeroVector, AttachComp, Volume, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

void USoundSubsystem::StopAllBGM()
{
	for (const auto& ActivatedBGM : ActivatedBGMComponents)
	{
		ActivatedBGM.Key->Stop();
		ActivatedBGM.Key->SetSound(nullptr);
	}
}

void USoundSubsystem::StopAllSFX()
{
	for (const auto& ActivatedSFX : ActivatedSFXComponents)
	{
		ActivatedSFX.Key->Stop();
		ActivatedSFX.Key->SetSound(nullptr);
	}
}

void USoundSubsystem::StopAllAmbient()
{
	for (const auto& ActivatedAmbient : ActivatedAmbientComponents)
	{
		ActivatedAmbient.Key->Stop();
		ActivatedAmbient.Key->SetSound(nullptr);
	}
}

void USoundSubsystem::StopAudio(const int32& AudioId, const bool& bShouldUseFadeOut, const float& FadeOutDuration, const float& FadeOutGoalVolume, const EAudioFaderCurve& FadeOutCurve)
{
	if (AudioComponentWithIdMap.Contains(AudioId) == false)
	{
		return;
	}

	UAudioComponent* AudioComponentWithId = AudioComponentWithIdMap[AudioId];
	if (IsValid(AudioComponentWithId) == false)
	{
		return;
	}

	if (bShouldUseFadeOut)
	{
		AudioComponentWithId->FadeOut(FadeOutDuration, FadeOutGoalVolume, FadeOutCurve);
	}
	else
	{
		AudioComponentWithId->Stop();
	}
}

bool USoundSubsystem::IsPlaying(const int32& AudioId)
{
	if (AudioComponentWithIdMap.Contains(AudioId) == false)
	{
		return false;
	}
	
	return AudioComponentWithIdMap[AudioId]->IsPlaying();;
}

float USoundSubsystem::GetAudioTotalDuration(const int32& AudioId) const
{
	if (AudioComponentWithIdMap.Contains(AudioId) == false)
	{
		return 0.0f;
	}

	return AudioComponentWithIdMap[AudioId]->Sound->GetDuration();

}

void USoundSubsystem::ChangeMasterVolume(const float& NewMasterVolume)
{
	MasterVolume = FMath::Clamp(NewMasterVolume, 0, 1);
	ChangeMasterVolumeInternal();
}

void USoundSubsystem::ChangeBGMVolume(const float& NewBGMVolume)
{
	BGMVolume = FMath::Clamp(NewBGMVolume, 0, 1);
	ChangeBGMVolumeInternal();
}

void USoundSubsystem::ChangeSFXVolume(const float& NewSFXVolume)
{
	SFXVolume = FMath::Clamp(NewSFXVolume, 0, 1);
	ChangeSFXVolumeInternal();
}

void USoundSubsystem::ChangeAmbientVolume(const float& NewAmbientVolume)
{
	AmbientVolume = FMath::Clamp(NewAmbientVolume, 0, 1);
	ChangeAmbientVolumeInternal();
}

void USoundSubsystem::ChangeMasterVolumeInternal()
{
	ChangeBGMVolumeInternal();
	ChangeSFXVolumeInternal();
	ChangeAmbientVolumeInternal();
}

void USoundSubsystem::ChangeBGMVolumeInternal()
{
	if (::IsValid(this) == false)
	{
		LOGV(Warning, TEXT("Not Valid"));
		return;
	}

	for (auto It = ActivatedBGMComponents.CreateIterator(); It; ++It)
	{
		if (IsValid(It->Key))
		{
			It->Key->SetVolumeMultiplier(MasterVolume * BGMVolume * It->Value);
		}
		else
		{
			LOGV(Warning, TEXT("Not Valid"));
			It.RemoveCurrent();
		}
	}
}

void USoundSubsystem::ChangeSFXVolumeInternal()
{
	if (::IsValid(this) == false)
	{
		LOGV(Warning, TEXT("Not Valid"));
		return;
	}

	for (auto It = ActivatedSFXComponents.CreateIterator(); It; ++It)
	{
		if (IsValid(It->Key))
		{
			It->Key->SetVolumeMultiplier(MasterVolume * SFXVolume * It->Value);
		}
		else
		{
			LOGV(Warning, TEXT("Not Valid"));
			It.RemoveCurrent();
		}
	}
}

void USoundSubsystem::ChangeAmbientVolumeInternal()
{
	if (::IsValid(this) == false)
	{
		LOGV(Warning, TEXT("Not Valid"));
		return;
	}

	for (auto It = ActivatedAmbientComponents.CreateIterator(); It; ++It)
	{
		if (IsValid(It->Key))
		{
			It->Key->SetVolumeMultiplier(MasterVolume * AmbientVolume * It->Value);
		}
		else
		{
			LOGV(Warning, TEXT("Not Valid"));
			It.RemoveCurrent();
		}
	}
}

void USoundSubsystem::OnAudioFinished(UAudioComponent* FinishedAudio)
{
	if (ActivatedBGMComponents.Contains(FinishedAudio))
	{
		ActivatedBGMComponents.Remove(FinishedAudio);
		LOGV(Log, TEXT("Removing BGM Comp, Activated : %d, Deactivated? : %d"), ActivatedBGMComponents.Num(), DeactivatedComponents.IsEmpty());
	}
	else if(ActivatedSFXComponents.Contains(FinishedAudio))
	{
		ActivatedSFXComponents.Remove(FinishedAudio);
		LOGV(Log, TEXT("Removing SFX Comp, Activated : %d, Deactivated? : %d"), ActivatedSFXComponents.Num(), DeactivatedComponents.IsEmpty());
	}
	else
	{
		ActivatedAmbientComponents.Remove(FinishedAudio);
		LOGV(Log, TEXT("Removing Ambient Comp, Activated : %d, Deactivated? : %d"), ActivatedAmbientComponents.Num(), DeactivatedComponents.IsEmpty());
	}

	if (::IsValid(FinishedAudio) == false || FinishedAudio->IsValidLowLevelFast() == false || FinishedAudio->IsValidLowLevel() == false)
	{
		LOGV(Log, TEXT("Trying to Deactivated, But Not Valid"));

		if (AudioIdWithComponentMap.Contains(FinishedAudio) == false)
		{
			return;
		}

		int32 OldId = AudioIdWithComponentMap[FinishedAudio];
		AudioComponentWithIdMap.Remove(OldId);
		AudioIdWithComponentMap.Remove(FinishedAudio);

		return;
	}
	
	if (FinishedAudio->GetAttachParent())
	{
		FDetachmentTransformRules Rule(EDetachmentRule::KeepWorld, false);
		FinishedAudio->DetachFromComponent(Rule);
		LOGV(Log, TEXT("Detached"));
	}

	FinishedAudio->OnAudioFinishedNative.RemoveAll(this);
	DeactivatedComponents.Enqueue(FinishedAudio);
	FinishedAudio->UnregisterComponent();
}

int32 USoundSubsystem::Play2DInternal(USoundBase* SoundAsset, const bool& bIsBGM, const bool& bIsAmbient, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	TObjectPtr<UAudioComponent> NewAudio = GetNewAudio();
	if (NewAudio == nullptr || IsValid(NewAudio) == false)
	{
		LOGV(Error, TEXT("Not Valid"));
		return INDEX_NONE;
	}

	float NewVolume = MasterVolume * Volume;

	if (bIsBGM)
	{
		check(ActivatedBGMComponents.Contains(NewAudio) == false);
		ActivatedBGMComponents.Add(NewAudio, Volume);
		NewVolume *= BGMVolume;
		LOGV(Log, TEXT("Play 2D Internal(BGM), Actvated : %d, Deactivated Empty? : %d"), ActivatedBGMComponents.Num(), DeactivatedComponents.IsEmpty());
	}
	else if (bIsAmbient)
	{
		check(ActivatedAmbientComponents.Contains(NewAudio) == false);
		ActivatedAmbientComponents.Add(NewAudio, Volume);
		NewVolume *= AmbientVolume;
		LOGV(Log, TEXT("Play 2D Internal(Ambient), Actvated : %d, Deactivated Empty? : %d"), ActivatedAmbientComponents.Num(), DeactivatedComponents.IsEmpty());
	}
	else
	{
		check(ActivatedSFXComponents.Contains(NewAudio) == false);
		ActivatedSFXComponents.Add(NewAudio, Volume);
		NewVolume *= SFXVolume;
		LOGV(Log, TEXT("Play 2D Internal(SFX), Actvated : %d, Deactivated Empty? : %d"), ActivatedSFXComponents.Num(), DeactivatedComponents.IsEmpty());
	}

	NewAudio->SetSound(SoundAsset);
	NewAudio->SetVolumeMultiplier(NewVolume);
	NewAudio->bAllowSpatialization = false;
	NewAudio->SetUISound(true);

	return PlayAudio(NewAudio, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::Play3DInternal(USoundBase* SoundAsset, const FVector& Position, USceneComponent* AttachComp, const float& Volume, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	TObjectPtr<UAudioComponent> NewAudio = GetNewAudio();
	if (NewAudio == nullptr || IsValid(NewAudio) == false)
	{
		LOGV(Error, TEXT("Not Valid"));
		return INDEX_NONE;
	}

	float NewVolume = MasterVolume * Volume;

	check(ActivatedSFXComponents.Contains(NewAudio) == false);
	ActivatedSFXComponents.Add(NewAudio, Volume);
	NewVolume *= SFXVolume;
	LOGV(Log, TEXT("Play 3D Internal, Actvated : %d, Deactivated Empty? : %d"), ActivatedSFXComponents.Num(), DeactivatedComponents.IsEmpty());

	NewAudio->SetSound(SoundAsset);
	NewAudio->SetVolumeMultiplier(NewVolume);

	NewAudio->bAllowSpatialization = true;
	NewAudio->SetUISound(false);
	NewAudio->SetWorldLocation(Position);
	LOGV(Log, TEXT("Located to : %s"), *Position.ToString());

	if (AttachComp)
	{
		FAttachmentTransformRules Rule(EAttachmentRule::KeepRelative, false);
		NewAudio->AttachToComponent(AttachComp, Rule);
		LOGV(Log, TEXT("Attached to : %s"), *AttachComp->GetName());
	}

	return PlayAudio(NewAudio, bShouldUseFadeIn, FadeInDuration, FadeInCurve);
}

int32 USoundSubsystem::PlayAudio(UAudioComponent* Audio, const bool& bShouldUseFadeIn, const float& FadeInDuration, const EAudioFaderCurve& FadeInCurve)
{
	if (AudioIdWithComponentMap.Contains(Audio) == false)
	{
		return INDEX_NONE;
	}

	Audio->OnAudioFinishedNative.AddUObject(this, &USoundSubsystem::OnAudioFinished);

	if (bShouldUseFadeIn)
	{
		Audio->FadeIn(FadeInDuration, 1.0f, 0.0f, FadeInCurve);
	}
	else
	{
		Audio->Play();
	}

	return AudioIdWithComponentMap[Audio];
}

UAudioComponent* USoundSubsystem::GetNewAudio()
{
	TObjectPtr<UAudioComponent> NewAudio = nullptr;

	UWorld* World = GetWorld();
	if (World == nullptr || World->bIsTearingDown)
	{
		return nullptr;
	}
	
	static const int32 SafetyLimit = 100;
	int32 Iteration = 0;
	for (; Iteration < SafetyLimit; ++Iteration)
	{
		if (DeactivatedComponents.IsEmpty())
		{
			CreateAudioComponent();
		}

		if (DeactivatedComponents.Dequeue(NewAudio) == false)
		{
			continue;
		}

		if (RemoveInvalidAudioComponent(NewAudio) == false)
		{
			if (NewAudio->IsRegistered() == false)
			{
				NewAudio->RegisterComponent();
			}

			break;
		}
	}

	if (Iteration >= SafetyLimit)
	{
		LOGV(Error, TEXT("Fail to Get New Audio"));
	}

	return NewAudio;
}

bool USoundSubsystem::RemoveInvalidAudioComponent(UAudioComponent* SomeAudio)
{
	UWorld* World = GetWorld();
	if (World == nullptr || World->bIsTearingDown)
	{
		return false;
	}

	if (SomeAudio && IsValid(SomeAudio))
	{
		return false;
	}

	if (AudioIdWithComponentMap.Contains(SomeAudio))
	{
		int32 OldId = AudioIdWithComponentMap[SomeAudio];
		AudioComponentWithIdMap.Remove(OldId);
		AudioIdWithComponentMap.Remove(SomeAudio);
	}

	return true;
}

void USoundSubsystem::CreateAudioComponent()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false || World->IsInSeamlessTravel() || World->bIsTearingDown || World->IsValidLowLevel() == false)
	{
		return;
	}

	AGameStateBase* GS = World->GetGameState();
	if (IsValid(GS) == false || GS->IsActorBeingDestroyed() || GS->IsValidLowLevel() == false)
	{
		return;
	}

	TObjectPtr<UAudioComponent> NewAudio = NewObject<UAudioComponent>((UObject*));
	if (IsValid(NewAudio) && NewAudio->IsValidLowLevel())
	{
		DeactivatedComponents.Enqueue(NewAudio);
		NewAudio->RegisterComponent();
		
		int32 NewId = CreateNewId();
		AudioComponentWithIdMap.Add(NewId, NewAudio);
		AudioIdWithComponentMap.Add(NewAudio, NewId);
	}

	return;
}

int32 USoundSubsystem::CreateNewId()
{
	int32 NewId = INDEX_NONE;

	static const int32 SafetyLimit = 100;
	int32 Iteration = 0;
	for (; Iteration < SafetyLimit; ++Iteration)
	{
		int32 TempId = FMath::Rand32();

		if (AudioComponentWithIdMap.Contains(NewId) == false)
		{
			NewId = TempId;
			break;
		}
	}

	if (Iteration >= SafetyLimit)
	{
		LOGV(Error, TEXT("Fail to Get New Id"));
	}

	return NewId;
}

void USoundSubsystem::OnWorldTearDown(UWorld* World)
{
	LOGV(Log, TEXT("World is Tearing Down"));

	StopAllBGM();
	StopAllAmbient();
	StopAllSFX();
	Init(0);
}

