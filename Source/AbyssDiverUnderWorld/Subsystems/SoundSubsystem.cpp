#include "Subsystems/SoundSubsystem.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADGameInstance.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void USoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

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

	LOGV(Warning, TEXT("SFXData is Initialized, Count : %d"), SFXDataCount);

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

	LOGV(Warning, TEXT("SFXBGMData is Initialized, Count : %d"), SFXBGMDataCount);

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

	LOGV(Warning, TEXT("SFXUIData is Initialized, Count : %d"), SFXUIDataCount);

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
	
	LOGV(Warning, TEXT("SFXMonsterData is Initialized, Count : %d"), SFXMonsterDataCount);

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

	LOGV(Warning, TEXT("AmbientData is Initialized, Count : %d"), AmbientDataCount);
}

void USoundSubsystem::Play2D(const ESFX& SFXType, const float& Volume)
{
	Play2DInternal(SFXData[int32(SFXType)]->Sound, false, false, Volume);
}

void USoundSubsystem::Play2D(const ESFX_Monster& SFXType, const float& Volume)
{
	Play2DInternal(SFXMonsterData[int32(SFXType)]->Sound, false, false, Volume);
}

void USoundSubsystem::Play2D(const ESFX_UI& SFXType, const float& Volume)
{
	Play2DInternal(SFXUIData[int32(SFXType)]->Sound, false, false, Volume);
}

void USoundSubsystem::PlayBGM(const ESFX_BGM& SFXType, const float& Volume)
{
	Play2DInternal(SFXBGMData[int32(SFXType)]->Sound, true, false, Volume);
}

void USoundSubsystem::PlayAmbient(const ESFX_Ambient& SFXType, const float& Volume)
{
	Play2DInternal(SFXBGMData[int32(SFXType)]->Sound, false, true, Volume);
}

void USoundSubsystem::PlayAt(const ESFX& SFXType, const FVector& Position, const float& Volume)
{
	Play3DInternal(SFXData[int32(SFXType)]->Sound, Position, nullptr, Volume);
}

void USoundSubsystem::PlayAt(const ESFX_Monster& SFXType, const FVector& Position, const float& Volume)
{
	Play3DInternal(SFXMonsterData[int32(SFXType)]->Sound, Position, nullptr, Volume);
}

void USoundSubsystem::PlayAt(const ESFX_UI& SFXType, const FVector& Position, const float& Volume)
{
	Play3DInternal(SFXUIData[int32(SFXType)]->Sound, Position, nullptr, Volume);
}

void USoundSubsystem::PlayAttach(const ESFX& SFXType, USceneComponent* AttachComp, const float& Volume)
{
	Play3DInternal(SFXData[int32(SFXType)]->Sound, FVector::ZeroVector, AttachComp, Volume);
}

void USoundSubsystem::PlayAttach(const ESFX_Monster& SFXType, USceneComponent* AttachComp, const float& Volume)
{
	Play3DInternal(SFXMonsterData[int32(SFXType)]->Sound, FVector::ZeroVector, AttachComp, Volume);
}

void USoundSubsystem::PlayAttach(const ESFX_UI& SFXType, USceneComponent* AttachComp, const float& Volume)
{
	Play3DInternal(SFXUIData[int32(SFXType)]->Sound, FVector::ZeroVector, AttachComp, Volume);
}

void USoundSubsystem::StopAllBGM()
{
	for (const auto& ActivatedBGM : ActivatedBGMComponents)
	{
		ActivatedBGM.Key->Stop();
	}
}

void USoundSubsystem::StopAllSFX()
{
	for (const auto& ActivatedSFX : ActivatedSFXComponents)
	{
		ActivatedSFX.Key->Stop();
	}
}

void USoundSubsystem::StopAllAmbient()
{
	for (const auto& ActivatedAmbient : ActivatedAmbientComponents)
	{
		ActivatedAmbient.Key->Stop();
	}
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
	for (const auto& BGM : ActivatedBGMComponents)
	{
		BGM.Key->SetVolumeMultiplier(MasterVolume * BGMVolume * BGM.Value);
	}
}

void USoundSubsystem::ChangeSFXVolumeInternal()
{
	for (const auto& SFX : ActivatedSFXComponents)
	{
		SFX.Key->SetVolumeMultiplier(MasterVolume * SFXVolume * SFX.Value);
	}
}

void USoundSubsystem::ChangeAmbientVolumeInternal()
{
	for (const auto& Ambient : ActivatedAmbientComponents)
	{
		Ambient.Key->SetVolumeMultiplier(MasterVolume * AmbientVolume * Ambient.Value);
	}
}

void USoundSubsystem::OnAudioFinished(UAudioComponent* FinishedAudio)
{
	if (ActivatedBGMComponents.Contains(FinishedAudio))
	{
		ActivatedBGMComponents.Remove(FinishedAudio);
		LOGV(Warning, TEXT("Removing BGM Comp, Activated : %d, Deactivated? : %d"), ActivatedBGMComponents.Num(), DeactivatedComponents.IsEmpty());
	}
	else if(ActivatedSFXComponents.Contains(FinishedAudio))
	{
		ActivatedSFXComponents.Remove(FinishedAudio);
		LOGV(Warning, TEXT("Removing SFX Comp, Activated : %d, Deactivated? : %d"), ActivatedSFXComponents.Num(), DeactivatedComponents.IsEmpty());
	}
	else
	{
		ActivatedAmbientComponents.Remove(FinishedAudio);
		LOGV(Warning, TEXT("Removing Ambient Comp, Activated : %d, Deactivated? : %d"), ActivatedAmbientComponents.Num(), DeactivatedComponents.IsEmpty());
	}

	if (::IsValid(FinishedAudio) == false || FinishedAudio->IsValidLowLevel() == false)
	{
		LOGV(Warning, TEXT("Trying to Deactivated, But Not Valid"));
		return;
	}
	
	if (FinishedAudio->GetAttachParent())
	{
		FDetachmentTransformRules Rule(EDetachmentRule::KeepWorld, false);
		FinishedAudio->DetachFromComponent(Rule);
		LOGV(Warning, TEXT("Detached"));
	}

	FinishedAudio->OnAudioFinishedNative.RemoveAll(this);
	DeactivatedComponents.Enqueue(FinishedAudio);
	FinishedAudio->UnregisterComponent();
}

void USoundSubsystem::Play2DInternal(USoundBase* SoundAsset, const bool& bIsBGM, const bool& bIsAmbient, const float& Volume)
{
	TObjectPtr<UAudioComponent> NewAudio = GetNewAudio();
	NewAudio->RegisterComponent();

	float NewVolume = MasterVolume * Volume;

	if (bIsBGM)
	{
		check(ActivatedBGMComponents.Contains(NewAudio) == false);
		ActivatedBGMComponents.Add(NewAudio, Volume);
		NewVolume *= BGMVolume;
		LOGV(Warning, TEXT("Play 2D Internal(BGM), Actvated : %d, Deactivated Empty? : %d"), ActivatedBGMComponents.Num(), DeactivatedComponents.IsEmpty());
	}
	else if (bIsAmbient)
	{
		check(ActivatedAmbientComponents.Contains(NewAudio) == false);
		ActivatedAmbientComponents.Add(NewAudio, Volume);
		NewVolume *= AmbientVolume;
		LOGV(Warning, TEXT("Play 2D Internal(Ambient), Actvated : %d, Deactivated Empty? : %d"), ActivatedAmbientComponents.Num(), DeactivatedComponents.IsEmpty());
	}
	else
	{
		check(ActivatedSFXComponents.Contains(NewAudio) == false);
		ActivatedSFXComponents.Add(NewAudio, Volume);
		NewVolume *= SFXVolume;
		LOGV(Warning, TEXT("Play 2D Internal(SFX), Actvated : %d, Deactivated Empty? : %d"), ActivatedSFXComponents.Num(), DeactivatedComponents.IsEmpty());
	}

	NewAudio->SetSound(SoundAsset);
	NewAudio->SetVolumeMultiplier(NewVolume);
	NewAudio->bAllowSpatialization = false;
	NewAudio->SetUISound(true);

	PlayAudio(NewAudio);
}

void USoundSubsystem::Play3DInternal(USoundBase* SoundAsset, const FVector& Position, USceneComponent* AttachComp, const float& Volume)
{
	TObjectPtr<UAudioComponent> NewAudio = GetNewAudio();
	NewAudio->RegisterComponent();

	float NewVolume = MasterVolume * Volume;

	check(ActivatedSFXComponents.Contains(NewAudio) == false);
	ActivatedSFXComponents.Add(NewAudio, Volume);
	NewVolume *= SFXVolume;
	LOGV(Warning, TEXT("Play 3D Internal, Actvated : %d, Deactivated Empty? : %d"), ActivatedSFXComponents.Num(), DeactivatedComponents.IsEmpty());

	NewAudio->SetSound(SoundAsset);
	NewAudio->SetVolumeMultiplier(NewVolume);

	NewAudio->bAllowSpatialization = true;
	NewAudio->SetUISound(false);
	NewAudio->SetWorldLocation(Position);
	LOGV(Warning, TEXT("Located to : %s"), *Position.ToString());

	if (AttachComp)
	{
		FAttachmentTransformRules Rule(EAttachmentRule::KeepRelative, false);
		NewAudio->AttachToComponent(AttachComp, Rule);
		LOGV(Warning, TEXT("Attached to : %s"), *AttachComp->GetName());
	}

	PlayAudio(NewAudio);
}

void USoundSubsystem::PlayAudio(UAudioComponent* Audio)
{
	Audio->OnAudioFinishedNative.AddUObject(this, &USoundSubsystem::OnAudioFinished);
	Audio->Play();
}

UAudioComponent* USoundSubsystem::GetNewAudio()
{
	TObjectPtr<UAudioComponent> NewAudio = nullptr;

	while (true)
	{
		if (DeactivatedComponents.IsEmpty())
		{
			NewAudio = NewObject<UAudioComponent>((UObject*)UGameplayStatics::GetGameState(GetWorld()));
			break;
		}
		else if (DeactivatedComponents.Dequeue(NewAudio) && ::IsValid(NewAudio) && NewAudio->IsValidLowLevel())
		{
			break;
		}
	}

	return NewAudio;
}
