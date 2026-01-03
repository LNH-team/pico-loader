.cpu arm946e-s
.section "patch_cardisetcarddma", "ax"
.syntax unified

.thumb
.global patch_cardisetcarddma_entry
.type patch_cardisetcarddma_entry, %function
patch_cardisetcarddma_entry:
    push {r4,lr}

    ldr r4, patch_cardisetcarddma_cardi_common
    ldr r0, [r4, #0x18] // src

    ldr r3, patch_cardisetcarddma_rom_offset_to_sd_sector_asm_address
    blx r3

    ldr r1, previousSector
    adr r2, previousSector
    str r0, [r2]
    ldr r2, [r4, #0x24] // dma channel
    ldr r3, [r4, #0x1C] // dst

    ldr r4, patch_cardisetcarddma_sdreaddma_asm_address
    blx r4

    pop {r4,pc}

.thumb
.global patch_cardionreadcard_entry
.type patch_cardionreadcard_entry, %function
patch_cardionreadcard_entry:
    push {lr}
    ldr r0,= 0x80000
    ldr r1, patch_cardionreadcard_osdisableirqmask_address
    blx r1

    ldr r0, patch_cardionreadcard_sdreaddma_finish_asm_address
    blx r0

    movs r0, #0
    adr r1, previousSector
    str r0, [r1]

    ldr r0,= 0x80000
    pop {r1}
    adds r1, #4
    bx r1

.balign 4

previousSector:
    .word 0

.global patch_cardisetcarddma_cardi_common
patch_cardisetcarddma_cardi_common:
    .word 0

.global patch_cardisetcarddma_rom_offset_to_sd_sector_asm_address
patch_cardisetcarddma_rom_offset_to_sd_sector_asm_address:
    .word 0

.global patch_cardisetcarddma_sdreaddma_asm_address
patch_cardisetcarddma_sdreaddma_asm_address:
    .word 0

.global patch_cardionreadcard_osdisableirqmask_address
patch_cardionreadcard_osdisableirqmask_address:
    .word 0

.global patch_cardionreadcard_sdreaddma_finish_asm_address
patch_cardionreadcard_sdreaddma_finish_asm_address:
    .word 0

.pool
.end
