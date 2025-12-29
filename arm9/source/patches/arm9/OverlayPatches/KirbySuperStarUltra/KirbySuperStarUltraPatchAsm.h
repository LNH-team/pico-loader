#pragma once
#include "sections.h"

DEFINE_SECTION_SYMBOLS(kirbyultrapatch);

extern "C" void kirbyultrapatch_entry();

extern u32 kirbyultrapatch_offset;

extern const void* kirbyultrapatch_nextAddress;  // Next patch address
