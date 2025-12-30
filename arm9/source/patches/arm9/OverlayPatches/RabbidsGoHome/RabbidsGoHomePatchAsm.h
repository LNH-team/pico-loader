#pragma once
#include "sections.h"

DEFINE_SECTION_SYMBOLS(rabbidspatch);

extern "C" void rabbidspatch_entry();

extern u32 rabbidspatch_offset;

extern const void* rabbidspatch_nextAddress;  // Next patch address
