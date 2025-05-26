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
}

void USoundSubsystem::Play2D(ESFX SFXType, float Volume)
{
	PlayInternal(SFXData[int32(SFXType)]->Sound, false, true, FVector::ZeroVector, nullptr, Volume);
}

void USoundSubsystem::Play2D(ESFX_Monster SFXType, float Volume)
{
	PlayInternal(SFXMonsterData[int32(SFXType)]->Sound, false, true, FVector::ZeroVector, nullptr, Volume);
}

void USoundSubsystem::Play2D(ESFX_UI SFXType, float Volume)
{
	PlayInternal(SFXUIData[int32(SFXType)]->Sound, false, true, FVector::ZeroVector, nullptr, Volume);
}

void USoundSubsystem::PlayBGM(ESFX_BGM SFXType, float Volume)
{
	PlayInternal(SFXBGMData[int32(SFXType)]->Sound, true, true, FVector::ZeroVector, nullptr, Volume);
}

void USoundSubsystem::PlayAt(ESFX SFXType, const FVector& Position, float Volume)
{
	PlayInternal(SFXData[int32(SFXType)]->Sound, false, false, Position, nullptr, Volume);
}

void USoundSubsystem::PlayAt(ESFX_Monster SFXType, const FVector& Position, float Volume)
{
	PlayInternal(SFXMonsterData[int32(SFXType)]->Sound, false, true, Position, nullptr, Volume);
}

void USoundSubsystem::PlayAt(ESFX_UI SFXType, const FVector& Position, float Volume)
{
	PlayInternal(SFXUIData[int32(SFXType)]->Sound, false, false, Position, nullptr, Volume);
}

void USoundSubsystem::PlayAttach(ESFX SFXType, USceneComponent* AttachComp, float Volume)
{
	PlayInternal(SFXData[int32(SFXType)]->Sound, false, false, FVector::ZeroVector, AttachComp, Volume);
}

void USoundSubsystem::PlayAttach(ESFX_Monster SFXType, USceneComponent* AttachComp, float Volume)
{
	PlayInternal(SFXMonsterData[int32(SFXType)]->Sound, false, false, FVector::ZeroVector, AttachComp, Volume);
}

void USoundSubsystem::PlayAttach(ESFX_UI SFXType, USceneComponent* AttachComp, float Volume)
{
	PlayInternal(SFXUIData[int32(SFXType)]->Sound, false, false, FVector::ZeroVector, AttachComp, Volume);
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

void USoundSubsystem::ChangeMasterVolume(float NewMasterVolume)
{
	MasterVolume = FMath::Clamp(NewMasterVolume, 0, 1);
	ChangeVolumeInternal();
}

void USoundSubsystem::ChangeBGMVolume(float NewBGMVolume)
{
	BGMVolume = FMath::Clamp(NewBGMVolume, 0, 1);
	ChangeVolumeInternal();
}

void USoundSubsystem::ChangeSFXVolume(float NewSFXVolume)
{
	SFXVolume = FMath::Clamp(NewSFXVolume, 0, 1);
	ChangeVolumeInternal();
}

void USoundSubsystem::ChangeVolumeInternal()
{
	for (const auto& BGM : ActivatedBGMComponents)
	{
		BGM.Key->SetVolumeMultiplier(MasterVolume * BGMVolume * BGM.Value);
	}

	for (const auto& SFX : ActivatedSFXComponents)
	{
		SFX.Key->SetVolumeMultiplier(MasterVolume * SFXVolume * SFX.Value);
	}
}

void USoundSubsystem::OnAudioFinished(UAudioComponent* FinishedAudio)
{
	bool bIsBGM = true;
	if (ActivatedBGMComponents.Contains(FinishedAudio))
	{
		ActivatedBGMComponents.Remove(FinishedAudio);
	}
	else
	{
		ActivatedSFXComponents.Remove(FinishedAudio);
		bIsBGM = false;
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

	LOGV(Warning, TEXT("OnAudioFinished, Actvated : %d, Deactivated Empty? : %d"), (bIsBGM ? ActivatedBGMComponents.Num() : ActivatedSFXComponents.Num()), DeactivatedComponents.IsEmpty());
}

void USoundSubsystem::PlayInternal(USoundBase* SoundAsset, bool bIsBGM, bool bIs2D, const FVector& Position, USceneComponent* AttachComp, float Volume)
{
	TObjectPtr<UAudioComponent> NewAudio = nullptr;

	while (true)
	{
		if (DeactivatedComponents.IsEmpty())
		{
			NewAudio = NewObject<UAudioComponent>((UObject*)UGameplayStatics::GetGameState(GetWorld()));
			break;
		}
		else if(DeactivatedComponents.Dequeue(NewAudio) && ::IsValid(NewAudio) && NewAudio->IsValidLowLevel())
		{
			break;
		}
	}
	
	NewAudio->RegisterComponent();

	float NewVolume = MasterVolume * Volume;

	if (bIsBGM)
	{
		check(ActivatedBGMComponents.Contains(NewAudio) == false);
		ActivatedBGMComponents.Add(NewAudio, Volume);
		NewVolume *= BGMVolume;
		LOGV(Warning, TEXT("Play Internal, Actvated : %d, Deactivated Empty? : %d"), ActivatedBGMComponents.Num(), DeactivatedComponents.IsEmpty());
	}
	else
	{
		check(ActivatedSFXComponents.Contains(NewAudio) == false);
		ActivatedSFXComponents.Add(NewAudio, Volume);
		NewVolume *= SFXVolume;
		LOGV(Warning, TEXT("Play Internal, Actvated : %d, Deactivated Empty? : %d"), ActivatedSFXComponents.Num(), DeactivatedComponents.IsEmpty());
	}

	NewAudio->SetSound(SoundAsset);
	NewAudio->SetVolumeMultiplier(NewVolume);

	if (bIs2D)
	{
		NewAudio->bAllowSpatialization = false;
		NewAudio->SetUISound(true);
	}
	else
	{
		NewAudio->bAllowSpatialization = true;
		NewAudio->SetUISound(false);
		NewAudio->SetWorldLocation(Position);
		LOGV(Warning, TEXT("Located to : %s"), *Position.ToString());
	}
	
	if (AttachComp)
	{
		FAttachmentTransformRules Rule(EAttachmentRule::KeepRelative, false);
		NewAudio->AttachToComponent(AttachComp, Rule);
		LOGV(Warning, TEXT("Attached to : %s"), *AttachComp->GetName());
	}

	NewAudio->OnAudioFinishedNative.AddUObject(this, &USoundSubsystem::OnAudioFinished);
	NewAudio->Play();
}
