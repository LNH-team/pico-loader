#pragma once
#include "sections.h"

DEFINE_SECTION_SYMBOLS(patch_carditaskthread);

extern "C" void __patch_carditaskthread_entry();
extern "C" void __patch_carditaskthread_entry_sdk4();

extern u16 __patch_carditaskthread_mov_cardicommon_to_r4;
extern u16 __patch_carditaskthread_mov_command_to_r0;
extern u16 __patch_carditaskthread_lsls_exmemstat_bit_to_r1;

extern u32 __patch_carditaskthread_failoffset;
extern u32 __patch_carditaskthread_successoffset;
extern u32 __patch_carditaskthread_readsave_asm_address;
extern u32 __patch_carditaskthread_writesave_asm_address;
extern u32 __patch_carditaskthread_verifysave_asm_address;
