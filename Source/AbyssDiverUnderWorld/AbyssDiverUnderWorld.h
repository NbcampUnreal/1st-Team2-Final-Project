// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define LOG_NETMODEINFO ((GetNetMode() == NM_Client) ? *FString::Printf(TEXT("Client%d"), UE::GetPlayInEditorID()) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDLALONE") : TEXT("SERVER")))
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define LOGN(Format, ...) UE_LOG(AbyssDiver, Warning, TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__));
#define LOG(Format, ...) UE_LOG(AbyssDiver, Warning, TEXT("%s %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__));
#define LOGV(Verbosity, Format, ...) UE_LOG(AbyssDiver, Verbosity, TEXT("%s(%s) %s"), LOG_CALLINFO, *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));
#define LOGVN(Verbosity, Format, ...) UE_LOG(AbyssDiver, Verbosity, TEXT("[%s] %s(%s) %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(AbyssDiver, Warning, All);