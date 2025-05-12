#include "Framework/ADGameInstance.h"
#include "SessionListWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

void UADGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UADGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UADGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UADGameInstance::OnJoinSessionComplete);
		}
	}
}

void UADGameInstance::CreateAdvancedSession(const FString& ServerName, bool bIsPrivate)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	if (!LocalPlayer || !LocalPlayer->GetPreferredUniqueNetId().IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid NetId found to create session."));
		return;
	}
	FUniqueNetIdRepl UserId = LocalPlayer->GetPreferredUniqueNetId();

	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = false;
	SessionSettings->NumPublicConnections = 4;
	SessionSettings->bShouldAdvertise = !bIsPrivate;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = bIsPrivate;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bIsDedicated = false;
	SessionSettings->bAllowInvites = true;

	SessionSettings->Set(FName("ServerName"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->CreateSession(*UserId, NAME_GameSession, *SessionSettings);
}

void UADGameInstance::FindSessions()
{

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = false;
			SessionSearch->MaxSearchResults = 50;
			SessionSearch->QuerySettings.Set(FName("Lobbies"), true, EOnlineComparisonOp::Equals);

			SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}

}

void UADGameInstance::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->JoinSession(0, NAME_GameSession, SearchResult);
	}
}

void UADGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName("Lobby_Test"), true, "listen");
	}
}

void UADGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Search Success: Success = %d, Result = %d"),
			bWasSuccessful, SessionSearch->SearchResults.Num());
	}

	if (SessionListWidgetClass)
	{
		if (!SessionListWidgetInstance)
		{
			SessionListWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), SessionListWidgetClass);
		}

		if (SessionListWidgetInstance && !SessionListWidgetInstance->IsInViewport())
		{
			SessionListWidgetInstance->AddToViewport();
			Cast<USessionListWidget>(SessionListWidgetInstance)->PopulateSessionList(SessionSearch->SearchResults);
		}
	}

}

void UADGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		return;
	}

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	FString JoinAddress;
	if (SessionInterface->GetResolvedConnectString(SessionName, JoinAddress))
	{
		UE_LOG(LogTemp, Log, TEXT("Join Session: address = %s"), *JoinAddress);
		APlayerController* PC = GetFirstLocalPlayerController();
		if (PC)
		{
			PC->ClientTravel(JoinAddress, TRAVEL_Absolute);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("세션 참가 실패: 주소를 가져올 수 없음"));
	}
}
