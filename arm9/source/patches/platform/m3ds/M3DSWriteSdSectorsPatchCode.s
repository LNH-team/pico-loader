.cpu arm7tdmi
.section "m3ds_writesdsectors", "ax"
.syntax unified
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global m3ds_writeSdSectors
.type m3ds_writeSdSectors, %function
m3ds_writeSdSectors:
    push {r1,r2,r4-r7,lr}
    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

    // construct first 4 bytes of the command
    // given a sector address in r0: 0xaabbccdd
    // first command word: B5 aa bb cc
    // B5 is for requesting the next sequential write
    adr r7, cmdWord0Tmp // write bytewise to this temp location
    movs r3, #0xB5
    strb r3, [r7,#0]
    lsrs r3, r0, #24
    strb r3, [r7,#1]
    lsrs r3, r0, #16
    strb r3, [r7,#2]
    lsrs r3, r0, #8
    strb r3, [r7,#3]
    ldr r6, [r7] // read back as 32 bit value from the temp location
    str r6, [r4,#8] // write to the actual command register

    // construct second 4 bytes of the command
    // second command word: dd 00 00 00
    lsls r0, r0, #24
    lsrs r7, r0, #24 // r7 = dd
    str r7, [r4,#0xC] // storing as little-endian puts the bottom 8 bits as first byte

    ldr r5, [sp,#4] // sector count
    movs r3, #0xBE // command for single sector write start
    movs r2, #0xBC // command for single sector write ready poll
    cmp r5, #1
    beq 1f
    movs r3, #0xB4 // command for multi sector write start
    movs r2, #0xB6 // command for multi sector write ready poll
1:
    mov r12, r2
    strb r3, [r4,#8] // use the right command for the first sector

    ldr r0, [sp,#0] // dst
sector_loop:
    ldr r3, m3ds_sendSector_address
    bl blx_r3
    bne sector_loop
    ldr r3, [sp,#4]
    cmp r3, #1
    bls write_sectors_end

    ldr r3, m3ds_sendSector_address
    adds r3, #(m3ds_finishMultiSectorWrite - m3ds_sendSector)
    bl blx_r3

write_sectors_end:
    pop {r1-r2,r4-r7,pc}

blx_r3:
    bx r3

.balign 4

.global m3ds_sendSector_address
m3ds_sendSector_address:
    .word 0

cmdWord0Tmp:
    .word 0

.pool

.section "m3ds_writesdsectors_sendSector", "ax"
.thumb
.global m3ds_sendSector
.type m3ds_sendSector, %function
m3ds_sendSector:
    ldr r1, =0x04100010

    ldr r3, =0xE1586100
    str r3, [r4,#4]

sendSector_data_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8 // check if ready to send
    bcc sendSector_data_loop_check_transfer_end // if not skip writing
    
    ldmia r0!, {r3}
    str r3, [r1]

sendSector_data_loop_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8 // check if transfer is done
    bcs sendSector_data_loop

    mov r3, r12 // command for write ready poll
    str r3, [r4,#8]
    movs r3, #0
    str r3, [r4,#0xC]
sendSector_poll_loop:
    ldr r3, =0xA7586000
    str r3, [r4,#4]

sendSector_poll_transfer_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8
    bcc sendSector_poll_check_transfer_end
    ldr r2, [r1]

sendSector_poll_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8
    bcs sendSector_poll_transfer_loop

    cmp r2, #0
    bne sendSector_poll_loop

    str r6, [r4,#8] // write the command for the next sector
    str r7, [r4,#0xC]
    subs r5, #1

    bx lr

m3ds_finishMultiSectorWrite:
    // send command for ending multi-sector write
    // B3 00 00 00 00 00 00 00
    movs r2, #0xB3
    str r2, [r4,#8] // B3 00 00 00
    str r5, [r4,#0xC] // 00 00 00 00
    ldr r3, =0xA0486100
    str r3, [r4,#4] // start transfer

    // wait for transfer to end
3:
    ldrb r1, [r4, #7]
    lsrs r1, r1, #8
    bcs 3b

    movs r3, #0xB6 // command for write ready poll
    str r3, [r4,#8]
    
    b sendSector_poll_loop

.balign 4

.pool

.end