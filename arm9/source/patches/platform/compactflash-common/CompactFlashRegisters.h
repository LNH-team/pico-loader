#pragma once
#include "common.h"

struct cf_registers_t
{
    vu16* data;
    vu16* altStatus;
    vu16* command;
    vu16* error;
    vu16* sectorCount;
    vu16* lba1;
    vu16* lba2;
    vu16* lba3;
    vu16* lba4;
};