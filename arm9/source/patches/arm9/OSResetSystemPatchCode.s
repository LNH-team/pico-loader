.cpu arm946e-s
.section "patch_osresetsystem", "ax"
.syntax unified

.thumb
.global patch_osresetsystem_entry
.type patch_osresetsystem_entry, %function
patch_osresetsystem_entry:
    movs r0, #0
    b patch_osresetsystem_entry_common

.global patch_osresetsystem_returnToLauncher
.type patch_osresetsystem_returnToLauncher, %function
patch_osresetsystem_returnToLauncher:
    movs r0, #1

patch_osresetsystem_entry_common:
    // r8 survives the SD read calls and tells part 2 whether this is a
    // normal SDK reset or an explicit return-to-launcher request.
    mov r8, r0

    adr r0, regIpcSync
    // regIpcSync, arm7ResetCommand, vramAbcdLcdcSetting, readSdSectors_address, loader_info_address, vramCLcdcAddress, patch_osresetsystem_bootPicoLoader_address
    ldmia r0, {r0, r1, r3, r4, r5, r6, r7}

    // r9 = readSdSectors_address
    mov r9, r4
    mov r10, r7

    // reset arm7
1:
    ldr r6, [r0, #4]
    lsrs r6, r6, #2 // shift bit 1 to carry
    bcs 1b // while fifo full

    str r1, [r0, #8]

    adds r7, #(twl_arm7_sync - patch_osresetsystem_bootPicoLoader)
.global patch_osresetsystem_entry_jump_to_twl_arm7_sync
patch_osresetsystem_entry_jump_to_twl_arm7_sync:
    blx r7

2:
    ldrb r7, [r0] // ipc sync
    cmp r7, #1
    bne 2b // while ipc sync from arm7 is not 1

    adds r0, #(0x04000240 - 0x04000180)

    // map vram ABCD to LCDC
    str r3, [r0]

    // load pico loader arm9
    ldmia r5!, {r4,r6,r7} // clusterShift, database, clusterMap[0]
    movs r7, #0x68
    lsls r7, r7, #20 // 0x06800000
    mov r11, pc
    b loadData

    // load pico loader arm7
    ldr r5, patch_osresetsystem_loader_info_address
    adds r5, #52
    ldr r7, vramCLcdcAddress
    mov r11, pc
    b loadData

    adr r5, patch_osresetsystem_loader_info_address
    bx r10

loadData_loop:
    subs r3, #2
    lsls r3, r4
    adds r0, r3, r6 // start sector
    movs r1, r7 // dst
    lsls r2, r4 // sector count
    lsls r3, r2, #9
    adds r7, r3

    blx r9 // read sectors

loadData:
    ldmia r5!, {r2, r3} // ncl, startSector
    cmp r2, #0
    bne loadData_loop
    mov pc, r11

.balign 4

regIpcSync:
    .word 0x04000180

arm7ResetCommand:
    .word 0x4000C

vramAbcdLcdcSetting:
    .word 0x80808080

.global patch_osresetsystem_readSdSectors_address
patch_osresetsystem_readSdSectors_address:
    .word 0

.global patch_osresetsystem_loader_info_address
patch_osresetsystem_loader_info_address:
    .word 0

vramCLcdcAddress:
    .word 0x06840000

.global patch_osresetsystem_bootPicoLoader_address
patch_osresetsystem_bootPicoLoader_address:
    .word 0

.pool

.section "patch_osresetsystem_boot", "ax"
.thumb
.global patch_osresetsystem_bootPicoLoader
.type patch_osresetsystem_bootPicoLoader, %function
patch_osresetsystem_bootPicoLoader:
    // loader_info_address, vramCLcdcAddress
    ldmia r5, {r3, r5}
    ldrh r0, [r3, #2] // loader_info_t::picoLoaderBootDrive
    strh r0, [r5, #8] // pload_header7_t::bootDrive

    // Preserve launcherPath in the freshly reloaded Pico Loader ARM7 image.
    // loader_info_t::launcherPath is at 0x70 and pload_header7_t::v2.launcherPath is at 0x310.
    movs r0, #0x70
    adds r0, r3
    movs r1, #0xC4
    lsls r1, r1, #2 // 0x310
    adds r2, r5, r1
    movs r4, #64
copyLauncherPath:
    ldr r6, [r0]
    str r6, [r2]
    adds r0, #4
    adds r2, #4
    subs r4, #1
    bne copyLauncherPath

    // For the hotkey path, launch the preserved launcher as a normal ROM instead
    // of setting the SDK-reset marker that would reload the current retail game.
    mov r1, r8
    cmp r1, #0
    beq setupBootState

    movs r0, #0x70
    adds r0, r3
    adds r2, r5, #0
    adds r2, #0x0C // pload_header7_t::loadParams.romPath
    movs r4, #64
copyLauncherToRomPath:
    ldr r6, [r0]
    str r6, [r2]
    adds r0, #4
    adds r2, #4
    subs r4, #1
    bne copyLauncherToRomPath

    // Clear save path and argv so the launcher starts as a clean normal launch.
    movs r0, #0x43
    lsls r0, r0, #2 // 0x10C = loadParams.savePath
    adds r0, r5
    movs r1, #0
    strb r1, [r0]

    movs r0, #0x83
    lsls r0, r0, #2 // 0x20C = loadParams.argumentsLength
    adds r0, r5
    str r1, [r0]
    strb r1, [r0, #4] // arguments[0]

setupBootState:
    adr r0, regVramCntA
    ldmia r0, {r0, r4, r6, r7}
    // r0 = regVramCntA
    // r4 = patch_osresetsystem_arm7Entry_address
    // r6 = vramCDSettings
    // r7 = patch_osresetsystem_cheats_address

    movs r2, #0x41
    lsls r2, r2, #4 // 0x410
    mov r1, r8
    cmp r1, #0
    beq storeCheats
    movs r7, #0 // Don't carry a retail game's cheats into the launcher.
storeCheats:
    str r7, [r5, r2] // pload_header7_t::v3.cheats

    // A normal SDK reset sets the marker consumed by arm7/main.cpp.
    // A hotkey return deliberately leaves the retail ARM7 entry untouched.
    mov r1, r8
    cmp r1, #0
    bne skipSdkResetMarker
    ldr r1, [r5] // pload_header7_t::entryPoint
    str r1, [r4] // NTR_SHARED_MEMORY->romHeader.arm7EntryAddress
skipSdkResetMarker:

    // map vram CD to arm7
    strh r6, [r0, #2]

    adds r0, #(0x04000180 - 0x04000240) // REG_IPC_SYNC
    movs r1, #1
    strb r1, [r0, #1]

1:
    ldrb r7, [r0] // ipc sync
    cmp r7, #0
    bne 1b // while ipc sync from arm7 is not 0

    movs r1, #0
    strb r1, [r0, #1]

    movs r0, #0x68
    lsls r0, r0, #20 // 0x06800000
    bx r0

twl_arm7_sync:
    ldr r7,= 0x02FFFC24
    movs r1, #1
    strh r1, [r7, #4]
    mov r11, pc
    b do_sync

    movs r1, #3
    strh r1, [r7, #4]
    mov r11, pc
    b do_sync

    mov pc, lr
do_sync:
    ldrh r6, [r7, #2]
    ldrh r1, [r7]
1:
    adds r1, #1
    strh r1, [r7]
    ldrh r4, [r7, #2]
    cmp r6, r4
    beq 1b
    adds r1, #1
    strh r1, [r7]
    mov pc, r11

.balign 4

regVramCntA:
    .word 0x04000240

.global patch_osresetsystem_arm7Entry_address
patch_osresetsystem_arm7Entry_address:
    .word 0x027FFE34

vramCDSettings:
    .word 0x8A82

.global patch_osresetsystem_cheats_address
patch_osresetsystem_cheats_address:
    .word 0

.pool
.end
