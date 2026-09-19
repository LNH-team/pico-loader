#pragma once
#include "loader/dldiHeader.h"

bool dldi_init();

/// @brief Keeps a copy of the card's DLDI driver on the mounted card.
///        A driver handed down through gLoaderHeader.dldiDriver is saved when the saved
///        copy is missing or belongs to a different driver. When no driver was handed down,
///        as after an in-game reset, the saved copy is loaded instead and is used by
///        dldi_patchTo() and dldi_copyTo().
/// @note Requires the card to be mounted.
void dldi_updateDriverCache();

/// @brief Returns whether a DLDI driver is available to patch into the program being booted,
///        either handed down through gLoaderHeader.dldiDriver or loaded from the card.
bool dldi_hasDriver();

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
