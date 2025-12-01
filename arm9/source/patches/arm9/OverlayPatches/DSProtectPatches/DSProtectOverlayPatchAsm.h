#pragma once
#include "sections.h"

DEFINE_SECTION_SYMBOLS(dsprotectpatch);

extern "C" void dsprotectpatch_entry();

extern u32 dsprotectpatch_patchType;    // 0=write dsprotectpatch_writeWord, 1=copy subsequent word
extern u32 dsprotectpatch_writeWord;    // Word to write if dsprotectpatch_patchType=0
extern u32 dsprotectpatch_offsetA1;     // Target offset for A1 (0xFFFFFFFF=invalid)
extern u32 dsprotectpatch_offsetNotA1;  // Target offset for NotA1 (0xFFFFFFFF=invalid)

extern u32 dsprotectpatch_overlayId;    // Target overlay ID
extern const void* dsprotectpatch_nextAddress;  // Next patch address
