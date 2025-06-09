#include "AbyssLogger.h"

FString AbyssLogger::FilePath;

void AbyssLogger::WriteLog(const FString& LogMessage)
{
#if UE_BUILD_SHIPPING && WRITE_LOG_WITH_SHIPPING

	if (FilePath.IsEmpty())
	{
		InitLogger();
	}

	const FDateTime Now = FDateTime::Now();
	const FString TimeStemp = Now.ToString(TEXT("[%Y-%m-%d %H:%M:%S] "));

	const FString NewLogMessage = TimeStemp + LogMessage + LINE_TERMINATOR;

	FFileHelper::SaveStringToFile(NewLogMessage, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);

#endif
}

void AbyssLogger::InitLogger()
{
#if UE_BUILD_SHIPPING && WRITE_LOG_WITH_SHIPPING

	const FDateTime Now = FDateTime::Now();
	const FString TimeStemp = Now.ToString(TEXT("[%Y%m%d_%H%M%S]"));
	const FString Directory = FPaths::ProjectSavedDir() + TEXT("AbyssLogger/");
	IFileManager::Get().MakeDirectory(*Directory, true);

	FilePath = Directory + FString::Printf(TEXT("AbyssShippingLog_%s.txt"), *TimeStemp);

#endif
}