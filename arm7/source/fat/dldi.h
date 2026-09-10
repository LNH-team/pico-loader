#pragma once
#include "dldiHeader.h"

bool dldi_init();
bool dldi_patchTo(dldi_header_t* stub);
void dldi_copyTo(void* target);

#ifdef __cplusplus
extern "C" {
#endif

bool dldi_readSectors(void* buffer, u32 sector, u32 count);
bool dldi_writeSectors(const void* buffer, u32 sector, u32 count);

#ifdef __cplusplus
}
#endif
