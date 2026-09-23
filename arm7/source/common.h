#pragma once
#include <nds/ndstypes.h>
#include "fat/ff.h"

#ifdef __cplusplus

#include "header.h"
#include "logger/ILogger.h"

extern ILogger* gLogger;

#define MAX_COMPILED_LOG_LEVEL  LogLevel::All

#define LOG_FATAL(...) if (LogLevel::Fatal < MAX_COMPILED_LOG_LEVEL) gLogger->Log(LogLevel::Fatal, __VA_ARGS__)
#define LOG_ERROR(...) if (LogLevel::Error < MAX_COMPILED_LOG_LEVEL) gLogger->Log(LogLevel::Error, __VA_ARGS__)
#define LOG_WARNING(...) if (LogLevel::Warning < MAX_COMPILED_LOG_LEVEL) gLogger->Log(LogLevel::Warning, __VA_ARGS__)
#define LOG_INFO(...) if (LogLevel::Info < MAX_COMPILED_LOG_LEVEL) gLogger->Log(LogLevel::Info, __VA_ARGS__)
#define LOG_DEBUG(...) if (LogLevel::Debug < MAX_COMPILED_LOG_LEVEL) gLogger->Log(LogLevel::Debug, __VA_ARGS__)
#define LOG_TRACE(...) if (LogLevel::Trace < MAX_COMPILED_LOG_LEVEL) gLogger->Log(LogLevel::Trace, __VA_ARGS__)

#endif

extern FATFS gFatFs;

#define STRINGIFY2(x)  #x
#define STRINGIFY(x)   STRINGIFY2(x)
