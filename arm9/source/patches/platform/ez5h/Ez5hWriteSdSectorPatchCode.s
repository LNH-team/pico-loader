#include "asminc.h"

.syntax unified
.thumb

.section "ez5h_crc", "ax"

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
@ void sdio_crc16_4bit_checksum(void* dataBuf, uint64_t* out)
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
@ 	*out = __builtin_bswap64(crc);
@ }

@ void sdio_crc16_4bit_checksum(void* inbuff, uint64_t* out)
@ args are passed on the stack:
@ inbuff = sp+4
@ out = sp
BEGIN_ASM_FUNC ez5h_sdio4BitCrc16
	ldr r0, [sp,#4]
	mov r1, sp
    push {r0,r2,r3,r4-r5,r6,lr}
    movs r4, #0 @ r4 = crc_lo
    movs r5, #0 @ r5 = crc_hi
    movs r6, #128
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

    subs r6, #1
    bne 1b

    movs r2, r4
    bl byteSwap32
	str r2, [r1,#4]

    movs r2, r5
    bl byteSwap32
	str r2, [r1]
    pop {r0,r2,r3,r4-r5,r6,pc}

byteSwap32:
    push {r4-r5,lr}
    movs r5, #16
    ldr r4, =0xFF00FF
    rors r2, r5 // ror 16
    ands r4, r2
    bics r2, r4
    lsls r4, r4, #8
    lsrs r2, r2, #8
    orrs r2, r4
    pop {r4-r5,pc}


.section "ez5h_write_data_rom_command", "ax"

@ez5h_sendWriteDataRomCommand(const u8* datab)
BEGIN_ASM_FUNC ez5h_sendWriteDataRomCommand
	ldrh r1, [r0]
	adds r0, #2
BEGIN_ASM_FUNC ez5h_sendWriteDataRomCommandShort
	push {r0,r3,lr}
	adr r0,send_writedata_data
	@ r0 holds EZ5H_CTRL_READ_0
	@ r2 holds the lower word of EZ5H_CMD_SDMC_WRITE_DATA 0xF6B8
	@ r3 holds REG_MCCNT0
	ldmia r0, {r0,r2-r3}

	@ REG_MCCNT0 + 8 = REG_MCCMD0, so offset all the next writes
	strh r2, [r3, #0+8]

	strb r1, [r3, #3+8]

	lsrs r1, #4
	strb r1, [r3, #2+8]

	lsrs r1, #4
	strb r1, [r3, #5+8]

	lsrs r1, #4
	strb r1, [r3, #4+8]
	
	@ REG_MCCNT0 is 0x040001A0, << 10 = 0xXXXX8000
	lsls r1, r3, #10
	strh r1, [r3]

	@ REG_MCCNT0 + 4 = REG_MCCNT1
	str r0, [r3, #4]

	@ check for busy
1:
	ldr r2, [r3, #4]
	@ check if bit 31 is set (busy flag)
	cmp r2, #0
	blt 1b
	pop {r0,r3,pc}

.balign 4
send_writedata_data:
	.word EZ5H_CTRL_READ_0
	.word 0xF6B8
	.word REG_MCCNT0


.section "ez5h_write_sector", "ax"

.global ez5h_sdhc_write_label

@ bool ez5h_writeSector(u32 sector, void* buffer)
BEGIN_ASM_FUNC ez5h_writeSector
	push {r0-r1,r4-r7,lr}

.global ez5h_sdhc_write_label
ez5h_sdhc_write_label:
	lsls r1, r0, #9

	movs r0, #0x58
	ldr r7, ez5h_writeSector_sendSDIOCommand
	bl write_trampoline	
	cmp r0, #0
	beq sdio_fail_write

	@ ez5h_sendSDIOCommand returned us EZ5H_CMD_SDMC_SEND_CLK(1) in r0-r1
	@ save low word of command
	movs r6, r0
	ldr r7, ez5h_writeSector_sendCommand
	bl write_trampoline	
	@ bl ez5h_sendCommand

	@ we use lower short as value to write, upper short is EZ5H_CMD_SDMC_SEND_CRC_STATUS used below
	ldr r1, =0xF8B8F0FF
	lsrs r5, r1, #16

	@ bl ez5h_sendWriteDataRomCommandShort
	ldr r7, ez5h_writeSector_sendWriteDataRomCommandShort
	bl write_trampoline	

	@ load buffer addr that was pushed at the start
	@ sdio4BitCrc16 will get the arguments directly from the stack
	@ and return the buffer address in r0
	@ ldr r0, [sp,#4]
	@ mov r1, sp
	@ bl ez5h_sdio4BitCrc16
	ldr r7, ez5h_writeSector_sdio4BitCrc16
	bl write_trampoline

	@ write the data
	@ r0 is the data buffer left untouched by the above function call
	@ and it gets automatically incremented in ez5h_sendWriteDataRomCommand
	movs r4, #0xFF
1:
	@ bl ez5h_sendWriteDataRomCommand
	ldr r7, ez5h_writeSector_sendWriteDataRomCommand
	bl write_trampoline
	@ do 0x100 iterations
	subs r4, #1
	bge 1b

	@ write the crc
	@ r0 gets automatically incremented in ez5h_sendWriteDataRomCommand
	mov r0, sp
	movs r4, #4
1:
	@ bl ez5h_sendWriteDataRomCommand
	ldr r7, ez5h_writeSector_sendWriteDataRomCommand
	bl write_trampoline
	subs r4, #1
	bne 1b

	@ wait crc status start acknowledgment
	@ load EZ5H_CMD_SDMC_SEND_CRC_STATUS
	movs r1, #0
	movs r0, r5
1:
	@ bl ez5h_sendCommand
	bl write_trampoline
	lsrs r2, #1
	bcs 1b

	@ send single crc read clock
	@ bl ez5h_sendCommand
	ldr r7, ez5h_writeSector_sendCommand
	bl write_trampoline

	@ wait crc status acknowledged
1:
	@ bl ez5h_sendCommand
	ldr r7, ez5h_writeSector_sendCommand
	bl write_trampoline
	lsrs r2, #1
	bcc 1b

	@ wait for card to be ready again
	@ load backed up EZ5H_CMD_SDMC_SEND_CLK(1), r1 is already setup as 0 from before
	movs r0, r6
	movs r4, #0xFF
1:
	@ bl ez5h_sendCommand
	ldr r7, ez5h_writeSector_sendCommand
	bl write_trampoline
	tst r2, r4
	bne 1b

sdio_fail_write:
	@ r0 either is 0 or is EZ5H_CMD_SDMC_SEND_CLK(1) (thus nonzero)
	pop	{r1-r2,r4-r7,pc}
write_trampoline:
	bx r7
.pool

.global ez5h_writeSector_sendSDIOCommand
.global ez5h_writeSector_sendCommand
.global ez5h_writeSector_sendWriteDataRomCommandShort
.global ez5h_writeSector_sendWriteDataRomCommand
.global ez5h_writeSector_sdio4BitCrc16

ez5h_writeSector_sendSDIOCommand:
	.word 0
ez5h_writeSector_sendCommand:
	.word 0
ez5h_writeSector_sendWriteDataRomCommandShort:
	.word 0
ez5h_writeSector_sendWriteDataRomCommand:
	.word 0
ez5h_writeSector_sdio4BitCrc16:
	.word 0
