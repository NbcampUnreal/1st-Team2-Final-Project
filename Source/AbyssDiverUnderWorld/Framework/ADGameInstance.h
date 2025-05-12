#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AdvancedFriendsGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ADGameInstance.generated.h"




UCLASS()
class ABYSSDIVERUNDERWORLD_API UADGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
    virtual void Init() override;

    UFUNCTION(BlueprintCallable)
    void CreateAdvancedSession(const FString& ServerName, bool bIsPrivate);

	UFUNCTION(BlueprintCallable)
    void FindSessions();

    void JoinSession(const FOnlineSessionSearchResult& SearchResult);

protected:
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnFindSessionsComplete(bool bWasSuccessful);

    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
protected:
    TSharedPtr<FOnlineSessionSettings> SessionSettings;

    TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> SessionListWidgetClass;

    UPROPERTY()
    UUserWidget* SessionListWidgetInstance;
};


