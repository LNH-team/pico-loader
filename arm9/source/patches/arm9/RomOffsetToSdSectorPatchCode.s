.cpu arm946e-s
.section "romoffsettosdsector", "ax"
.syntax unified
.thumb

// r0 = rom offset
// returns sd sector in r0
// returns cluster shift in r1
// returns remaining sectors in cluster in lr
.global rom_offset_to_sd_sector_asm
.type rom_offset_to_sd_sector_asm, %function
rom_offset_to_sd_sector_asm:
    push {r4,r5,lr}
retry:
    ldr r5, romoffsettosdsector_fatDataPtr
    ldmia r5!, {r1,r2,r3,r4} // clusterShift, database, clusterMask, clusterMap[0]
    lsrs r0, r0, #9
2:
    adds r4, r3, #1
    ands r3, r0
    subs r4, r3
    mov lr, r4 // remaining sectors in cluster
    adds r3, r2
    lsrs r0, r1 // cl
1:
    ldmia r5!, {r2,r4} // ncl, startSector
    cmp r2, #0
    beq out_of_bounds
    subs r0, r2 // cl -= ncl
    bcs 1b

    adds r0, r2
    adds r0, r4 // + startSector
    subs r0, #2 // - 2
    lsls r0, r1
    adds r0, r3 // r0 = src sector
    pop {r4,r5,pc}

out_of_bounds:
    movs r0, #0x40
    lsls r0, r0, #9
    b retry

.balign 4

.global romoffsettosdsector_fatDataPtr
romoffsettosdsector_fatDataPtr:
    .word 0x027FF800

.pool

.end