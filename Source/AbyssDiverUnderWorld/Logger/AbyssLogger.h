#pragma once

#include "CoreMinimal.h"

#define WRITE_LOG_WITH_SHIPPING 1

#if UE_BUILD_SHIPPING && WRITE_LOG_WITH_SHIPPING

#define AbyssLog(Format, ...) \
do { \
        const FString NewLogMessage = FString::Printf(TEXT("%s(%s) %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__)) \
        AbyssLogger::WriteLog(LogMessage); \
} while (0)

#else

#define AbyssLog(Format, ...)

#endif
/**
 * 
 */
class ABYSSDIVERUNDERWORLD_API AbyssLogger
{
private:

    AbyssLogger() = delete;
    ~AbyssLogger() = delete;
    AbyssLogger(const AbyssLogger&) = delete;
    AbyssLogger& operator=(const AbyssLogger&) = delete;

public:

    static void WriteLog(const FString& LogMessage);

private:

    static void InitLogger();

private:

    static FString FilePath;

};
