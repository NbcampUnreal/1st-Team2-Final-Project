#include "Framework/VOIPLIB.h"
#include "AbyssDiverUnderWorld.h"

void UVOIPLIB::ClearVoicePackets(UObject* WorldContextObject)
{
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World))
	{
		return;
	}

	IOnlineVoicePtr VoiceInterface = Online::GetVoiceInterface(World);

	if (!VoiceInterface.IsValid())
	{
		LOG(TEXT("Clear voice packets couldn't get the voie interface!"));
	}

	VoiceInterface->ClearVoicePackets();

}
