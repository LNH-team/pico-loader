.cpu arm7tdmi
.syntax unified
.section "scsd_change_mode", "ax"

#include "asminc.h"

.equ sd_resetaddr,   0x9440000

@ void sc_change_mode(uint16_t mode);
BEGIN_THUMB_FUNCTION sccmn_changeMode
    ldr r2,= 0x09FFFFFE
    ldr r3,= 0xA55A
    strh r3, [r2]
    strh r3, [r2]
    strh r0, [r2]
    strh r0, [r2]
	cmp r0, #3
	bne 1f
	@ if mode is 3 (sd enable), reset the sd card
    @ SDResetCard
    @ write 0
    ldr r2,= sd_resetaddr
    strh r2, [r2]
1:
    mov pc, lr

.section "scsd_common", "ax"
@ this function will trash r4 but leave r0 and r1 untouched
@ void SDSendClock10(void)
BEGIN_THUMB_FUNCTION sccmn_sdSendClock10
    movs r3, #0x10
    @ loads reg_scsd_cmd
    movs r2, #0x98
    lsls r2, r2, #20
1:
    ldrh r4, [r2]
    subs r3, r3, #1
    bne 1b
    mov pc, lr

@ static uint64_t inline calSingleCRC16(uint64_t crc, uint32_t data_in){
@ 	// Shift out 8 bits for each line
@ 	uint32_t data_out = crc >> 32;
@ 	crc <<= 32;
@
@ 	// XOR outgoing data to itself with 4 bit delay
@ 	data_out ^= (data_out >> 16);
@
@ 	// XOR incoming data to outgoing data with 4 bit delay
@ 	data_out ^= (data_in >> 16);
@
@ 	// XOR outgoing and incoming data to accumulator at each tap
@ 	uint64_t xorred = data_out ^ data_in;
@ 	crc ^= xorred;
@ 	crc ^= xorred << (5 * 4);
@ 	crc ^= xorred << (12 * 4);
@ 	return crc;
@ }
@ uint64_t sdio_crc16_4bit_checksum(void* dataBuf)
@ {
@ 	uint32_t num_words = 512 / sizeof(uint32_t);
@ 	uint64_t crc = 0;
@     auto* data = static_cast<uint32_t*>(dataBuf);
@     auto* end = data + num_words;
@     while (data < end)
@     {
@         uint32_t data_in = __builtin_bswap32(*data++);
@         crc = calSingleCRC16(crc, data_in);
@     }
@
@ 	return __builtin_bswap64(crc);
@ }

@ uint64_t sdio_crc16_4bit_checksum(void*)
@ r0 and r1 are left untouched, the result is returned in r2-r3 instead
BEGIN_THUMB_FUNCTION sccmn_sdio4BitCrc16
    push {r0,r1,r4-r5,lr}
    movs r4, #0 @ r4 = crc_lo
    movs r5, #0 @ r5 = crc_hi
    movs r1, #128
1:
    @ r5 = data_out
    lsrs r3, r5, #16
    eors r5, r3

    ldmia r0!, {r2}

    bl byteSwap32
    @ r2 = data_in

    lsrs r3, r2, #16
    eors r5, r3
    eors r2, r5 // r2 = xorred
    movs r5, r4 // r5 = crc_hi
    movs r4, r2 // r4 = crc_lo

    lsls r3, r2, #20
    eors r4, r3
    lsrs r3, r2, #12
    eors r5, r3
    lsls r3, r2, #16
    eors r5, r3

    subs r1, #1
    bne 1b

    movs r2, r4
    bl byteSwap32
    movs r3, r2

    movs r2, r5
    @falls through to perform a "final" byteSwap32 call
    b byteSwap32_nopush

byteSwap32:
    push {r0,r1,r4-r5,lr}
byteSwap32_nopush:
    movs r5, #16
    ldr r4,= 0xFF00FF
    rors r2, r5 // ror 16
    ands r4, r2
    bics r2, r4
    lsls r4, r4, #8
    lsrs r2, r2, #8
    orrs r2, r4
    pop {r0,r1,r4-r5,pc}

.balign 4

.pool
