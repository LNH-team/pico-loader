#pragma once
#include "sections.h"

DEFINE_SECTION_SYMBOLS(fsstartoverlayhook);

extern "C" void fsstartoverlayhook_entry();

extern u32 fsstartoverlayhook_dcFlushRangeAddress;
extern u32 fsstartoverlayhook_hookFuncAddress;
