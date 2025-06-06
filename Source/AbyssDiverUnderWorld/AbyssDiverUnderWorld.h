// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Logger/AbyssLogger.h"

#define LOG_NETMODEINFO ((GetNetMode() == NM_Client) ? *FString::Printf(TEXT("Client%d"), UE::GetPlayInEditorID()) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDLALONE") : TEXT("SERVER")))
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

#if UE_BUILD_SHIPPING && WRITE_LOG_WITH_SHIPPING

#define LOGN(Format, ...) \
do{ \
	const FString Message = FString::Printf(TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__)); \
	AbyssLogger::WriteLog(Message); \
} while (0);

#define LOG(Format, ...) \
do{ \
	const FString Message = FString::Printf(TEXT("%s %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__)); \
	AbyssLogger::WriteLog(Message); \
} while (0);

#define LOGV(Verbosity, Format, ...) \
do {\
	const FString Message = FString::Printf(TEXT("%s(%s) %s"), LOG_CALLINFO, *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__)); \
	AbyssLogger::WriteLog(Message); \
} while (0);

#define LOGVN(Verbosity, Format, ...) \
do{\
	const FString Message = FString::Printf(TEXT("[%s] %s(%s) %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__)); \
	AbyssLogger::WriteLog(Message); \
} while (0);

#else

#define LOGN(Format, ...) UE_LOG(AbyssDiver, Warning, TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__));
#define LOG(Format, ...) UE_LOG(AbyssDiver, Warning, TEXT("%s %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__));
#define LOGV(Verbosity, Format, ...) UE_LOG(AbyssDiver, Verbosity, TEXT("%s(%s) %s"), LOG_CALLINFO, *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));
#define LOGVN(Verbosity, Format, ...) UE_LOG(AbyssDiver, Verbosity, TEXT("[%s] %s(%s) %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

#endif

DECLARE_LOG_CATEGORY_EXTERN(AbyssDiver, Log, All);