#pragma once
#include "common.h"

struct cf_registers_t
{
    u32 data;
    u32 altStatus;
    u32 command;
    u32 error;
    u32 sectorCount;
    u32 lba1;
    u32 lba2;
    u32 lba3;
    u32 lba4;
};