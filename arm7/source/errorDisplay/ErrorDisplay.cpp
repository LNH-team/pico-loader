#include "common.h"
#include <stdarg.h>
#include <libtwl/i2c/i2cMcu.h>
#include "ipc.h"
#include "ipcCommands.h"
#include "core/mini-printf.h"
#include "ErrorDisplay.h"

void ErrorDisplay::PrintWarning(const char* errorFormat, ...)
{
    va_list va;
    va_start(va, errorFormat);
    mini_vsnprintf((char*)0x02000000, 1024, errorFormat, va);
    sendToArm9(IPC_COMMAND_ARM9_DISPLAY_ERROR);
    sendToArm9(1);
    receiveFromArm9();
    va_end(va);
}

void ErrorDisplay::PrintError(const char* errorFormat, ...)
{
    va_list va;
    va_start(va, errorFormat);
    mini_vsnprintf((char*)0x02000000, 1024, errorFormat, va);
    sendToArm9(IPC_COMMAND_ARM9_DISPLAY_ERROR);
    sendToArm9(0);
    va_end(va);
    while (true);
}
