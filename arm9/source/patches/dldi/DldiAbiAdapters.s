.cpu arm7tdmi
.syntax unified

// ---------------------------------------------------------------------
// DLDI ABI adapters - platform independent.
//
// Every supported flashcart provides its SD access code through the same
// interface (see IReadSectorsPatchCode.h / IWriteSectorsPatchCode.h):
//
//     void readSectors(u32 srcSector, void* dst, u32 sectorCount)
//
// That code exists to be injected into a retail game, replacing the game's
// own card read. The DLDI ABI a homebrew stub's function pointers must
// satisfy is different (see dldiHeader.h):
//
//     bool dldiReadSectors(u32 sector, u32 count, void* dst)
//
// - arguments 2 and 3 swapped, and a boolean success return expected.
//
// These adapters reorder the arguments, call through to the platform's real
// code via a runtime-filled address slot, and always report success (matching
// the underlying code, which does not distinguish SD errors at this layer).
// Nothing here is specific to any one cartridge: the target address is filled
// in from whatever sLoaderPlatform->CreateSdReadPatchCode() returns, so one
// copy of this glue serves every supported platform.
//
// INTERWORKING: these adapters must not return with "pop {pc}".
//
// They are Thumb. On ARMv4T (the ARM7TDMI) "pop {pc}" writes PC without
// restoring the T bit, so it only returns correctly when the caller was also
// Thumb. A driver patched into a stub is reached through the stub's function
// pointers by arbitrary homebrew, which may be built for either instruction
// set - a caller built -marm is entirely normal. Returning with "pop {pc}"
// to such a caller leaves the core in Thumb state decoding ARM instructions.
//
// Annotating the caller instead - forcing it to Thumb - is not sound here: it
// only covers call sites this codebase owns, and these entry points are
// invoked by code it does not.
//
// "pop {r4} / pop {r3} / bx r3" restores the T bit and is correct from either
// state.
//
// The platform read/write code these call into returns to this adapter, which
// is Thumb, so its own "pop {pc}" is safe.
// ---------------------------------------------------------------------

.section "dldi_abi_read_adapter", "ax"
.thumb

// DLDI ABI in:  r0=sector, r1=count, r2=dst
// Real code in: r0=sector, r1=dst,   r2=count
.global dldi_abi_readSectorsAdapter
.type dldi_abi_readSectorsAdapter, %function
dldi_abi_readSectorsAdapter:
    push {r4, lr}
    movs r4, r1                      // r4 = count (saved)
    movs r1, r2                      // r1 = dst
    movs r2, r4                      // r2 = count
    ldr r3, dldi_abi_read_target
    bl dldi_abi_read_blx_r3
    movs r0, #1                      // DLDI success
    // Interworking return - see the note at the top of this file.
    pop {r4}
    pop {r3}
    bx r3

dldi_abi_read_blx_r3:
    bx r3

.balign 4

.global dldi_abi_read_target
dldi_abi_read_target:
    .word 0

.pool

.section "dldi_abi_write_adapter", "ax"
.thumb

// DLDI ABI in:  r0=sector, r1=count, r2=src
// Real code in: r0=sector, r1=src,   r2=count
.global dldi_abi_writeSectorsAdapter
.type dldi_abi_writeSectorsAdapter, %function
dldi_abi_writeSectorsAdapter:
    push {r4, lr}
    movs r4, r1                      // r4 = count (saved)
    movs r1, r2                      // r1 = src
    movs r2, r4                      // r2 = count
    ldr r3, dldi_abi_write_target
    bl dldi_abi_write_blx_r3
    movs r0, #1                      // DLDI success
    // Interworking return - see the note at the top of this file.
    pop {r4}
    pop {r3}
    bx r3

dldi_abi_write_blx_r3:
    bx r3

.balign 4

.global dldi_abi_write_target
dldi_abi_write_target:
    .word 0

.pool

.section "dldi_abi_trivial_stub", "ax"
.thumb

// Trivial "always succeed" implementation, used for the DLDI functions
// that don't need real behavior in this fallback driver (startup,
// isInserted, clearStatus, shutdown). All four point at this same stub.
.global dldi_abi_trivialStub
.type dldi_abi_trivialStub, %function
dldi_abi_trivialStub:
    movs r0, #1
    bx lr

.pool
.end
