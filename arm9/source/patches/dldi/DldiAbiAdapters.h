#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../platform/IReadSectorsPatchCode.h"
#include "../platform/IWriteSectorsPatchCode.h"

// See DldiAbiAdapters.s for the full explanation of why these exist:
// they reorder arguments and normalize the return value so the real
// generated SD access code can be exposed as a standard DLDI driver
// function, for the "no valid driver was handed down" boot path.

DEFINE_SECTION_SYMBOLS(dldi_abi_read_adapter);
DEFINE_SECTION_SYMBOLS(dldi_abi_write_adapter);
DEFINE_SECTION_SYMBOLS(dldi_abi_trivial_stub);

extern "C" void dldi_abi_readSectorsAdapter();
extern "C" void dldi_abi_writeSectorsAdapter();
extern "C" void dldi_abi_trivialStub();
extern u32 dldi_abi_read_target;
extern u32 dldi_abi_write_target;

/// @brief Wraps the real (already-placed) SD read patch code with a
/// DLDI-ABI-compatible entry point, following the same runtime-filled-
/// address-slot idiom DSPicoReadSdSectorsPatchCode already uses
/// internally for its own indirect call.
class DldiAbiReadAdapterPatchCode : public PatchCode
{
public:
    DldiAbiReadAdapterPatchCode(PatchHeap& patchHeap, const IReadSectorsPatchCode* realReadPatchCode)
        : PatchCode(SECTION_START(dldi_abi_read_adapter), SECTION_SIZE(dldi_abi_read_adapter), patchHeap)
    {
        dldi_abi_read_target = (u32)realReadPatchCode->GetReadSectorsFunction();
    }

    /// @brief Address to use as a dldi_header_t::readSectorsFuncAddress.
    u32 GetDldiReadSectorsFuncAddress() const
    {
        return (u32)GetAddressAtTarget((void*)dldi_abi_readSectorsAdapter);
    }
};

/// @brief Wraps the real (already-placed) SD write patch code with a
/// DLDI-ABI-compatible entry point. Same idiom as the read adapter above.
class DldiAbiWriteAdapterPatchCode : public PatchCode
{
public:
    DldiAbiWriteAdapterPatchCode(PatchHeap& patchHeap, const IWriteSectorsPatchCode* realWritePatchCode)
        : PatchCode(SECTION_START(dldi_abi_write_adapter), SECTION_SIZE(dldi_abi_write_adapter), patchHeap)
    {
        dldi_abi_write_target = (u32)realWritePatchCode->GetWriteSectorFunction();
    }

    /// @brief Address to use as a dldi_header_t::writeSectorsFuncAddress.
    u32 GetDldiWriteSectorsFuncAddress() const
    {
        return (u32)GetAddressAtTarget((void*)dldi_abi_writeSectorsAdapter);
    }
};

/// @brief Trivial "always succeed" function, used for the DLDI functions
/// that don't need real behavior in this fallback driver (startup,
/// isInserted, clearStatus, shutdown).
class DldiAbiTrivialStubPatchCode : public PatchCode
{
public:
    explicit DldiAbiTrivialStubPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dldi_abi_trivial_stub), SECTION_SIZE(dldi_abi_trivial_stub), patchHeap) { }

    /// @brief Address to use for dldi_header_t::startupFuncAddress etc.
    u32 GetDldiTrivialFuncAddress() const
    {
        return (u32)GetAddressAtTarget((void*)dldi_abi_trivialStub);
    }
};
