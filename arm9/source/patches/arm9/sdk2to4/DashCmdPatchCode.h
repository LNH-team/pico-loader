#pragma once
#include "common.h"
#include "sections.h"

DEFINE_SECTION_SYMBOLS(dash_cmd_fix);

extern "C" void dash_cmd_patch_entry();
extern "C" void dash_finish_entry();
extern "C" u32 dbr_fixcp15;
extern "C" u32 dbr_remap;
extern "C" u32 dbr_global_ptr;
extern "C" u32 dbr_rsd;
extern "C" u32 dbr_finish;
