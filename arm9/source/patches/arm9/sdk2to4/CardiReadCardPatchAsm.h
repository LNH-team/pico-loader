#pragma once
#include "sections.h"

DEFINE_SECTION_SYMBOLS(patch_cardireadcard);

extern "C" void patch_cardireadcard_entry();

extern u32 __patch_cardireadcard_fix_cp15_asm_address;
extern u32 __patch_cardireadcard_rom_offset_to_sd_sector_asm_address;
extern u32 __patch_cardireadcard_sdread_asm_address;

extern u16 patch_cardireadcard_return_offset;
extern u16 patch_cardireadcard_mov_src_to_r0;
extern u16 patch_cardireadcard_mov_dst_to_r1;
extern u16 patch_cardireadcard_mov_cardicommon_to_r6;
extern u16 patch_cardireadcard_adjust_cardicommon_offset;
extern u16 patch_cardireadcard_mov_r3_to_dst;

// Unmodified upstream read-only template for fragmented patch heaps.
DEFINE_SECTION_SYMBOLS(patch_cardireadcard_compat);
extern "C" void patch_cardireadcard_compat_entry();
extern u16 patch_cardireadcard_compat_return_offset;
extern u16 patch_cardireadcard_compat_mov_src_to_r0;
extern u16 patch_cardireadcard_compat_mov_dst_to_r1;
extern u16 patch_cardireadcard_compat_mov_cardicommon_to_r6;
extern u16 patch_cardireadcard_compat_adjust_cardicommon_offset;
extern u16 patch_cardireadcard_compat_mov_r3_to_dst;
extern u32 __patch_cardireadcard_compat_fix_cp15_asm_address;
extern u32 __patch_cardireadcard_compat_rom_offset_to_sd_sector_asm_address;
extern u32 __patch_cardireadcard_compat_sdread_asm_address;

// Unmodified upstream read-only template for fragmented patch heaps.
DEFINE_SECTION_SYMBOLS(patch_cardireadcard_split);
extern "C" void patch_cardireadcard_split_entry();
extern u16 patch_cardireadcard_split_return_offset;
extern u16 patch_cardireadcard_split_mov_src_to_r0;
extern u16 patch_cardireadcard_split_mov_dst_to_r1;
extern u16 patch_cardireadcard_split_mov_cardicommon_to_r6;
extern u16 patch_cardireadcard_split_adjust_cardicommon_offset;
extern u16 patch_cardireadcard_split_mov_r3_to_dst;
extern u32 __patch_cardireadcard_split_fix_cp15_asm_address;
extern u32 __patch_cardireadcard_split_rom_offset_to_sd_sector_asm_address;
extern u32 __patch_cardireadcard_split_sdread_asm_address;

DEFINE_SECTION_SYMBOLS(patch_split_ack);
DEFINE_SECTION_SYMBOLS(patch_split_park);
DEFINE_SECTION_SYMBOLS(patch_split_load);
DEFINE_SECTION_SYMBOLS(patch_split_check);
extern "C" void splitAckEntry();
extern "C" void splitParkEntry();
extern "C" void splitLoadEntry();
extern "C" void splitCheckEntry();
extern "C" u32 splitReadNext;
extern "C" u32 splitAckNext;
extern "C" u32 splitParkLoad;
extern "C" u32 splitLoadExtents;
extern "C" u32 splitLoadRead;
extern "C" u32 splitLoadCheck;
