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
@ 	auto* data = static_cast<uint32_t*>(dataBuf);
@ 	auto* end = data + num_words;
@ 	while (data < end)
@ 	{
@ 	    uint32_t data_in = __builtin_bswap32(*data++);
@       crc = calSingleCRC16(crc, data_in);
@ 	}
@
@ 	*out = __builtin_bswap64(crc);
@ }

@ void sdio_crc16_4bit_checksum(void* inbuff, uint64_t* out)
@ args are passed on the stack:
@ inbuff = sp+4
@ out = sp
@
@ returns:
@	in r0 the value at sp+4 passed in input
@	in sp+0/sp+4 the crc value ready to be sent

BEGIN_ASM_FUNC ez5h_sdio4BitCrc16
	ldr r0, [sp,#4]
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
	@ write return high part to the stack slot sp+4 (which gets offsetted by 28 due to 7 extra regs having been pushed)
	str r2, [sp,#4+28]

	movs r2, r5
	bl byteSwap32
	@ write return high part to the stack slot sp+0 (which gets offsetted by 28 due to 7 extra regs having been pushed)
	str r2, [sp,#0+28]
	@ r7 used as scratch
	pop {r0,r2,r3,r4-r5,r6,r7}
	mov pc,r7

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


.section "ez5h_write_multiple_sector", "ax"

.global ez5h_writeMultipleSector_writeSector_addr
.global ez5h_writeMultipleSector_doSDOperation
.global ez5h_writeMultipleSector_sendCommand
.global ez5h_writeMultipleSector_sdio4BitCrc16
.arm
@ez5h_writeMultipleSector(u32 sector, u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC ez5h_writeMultipleSector
	push {r4-r7,r8-r12,lr}
	adr SEND_WRITE_DATA_ROM_REG, sdio_functions
	ldmia SEND_WRITE_DATA_ROM_REG!, {r3,r7,SEND_COMMAND_REG,SDIO_CRC_REG}
	@ doSDOperation will pop r4-r7 off the stack
	@ leaving to us to pop the remaining hiregs
	bl trampoline
	pop {r8-r12,lr}
	bx lr
trampoline:
	bx r7

sdio_functions:
ez5h_writeMultipleSector_writeSector_addr:
	.word 0
ez5h_writeMultipleSector_doSDOperation:
	.word 0
ez5h_writeMultipleSector_sendCommand:
	.word 0
ez5h_writeMultipleSector_sdio4BitCrc16:
	.word 0

.thumb
@ez5h_sendWriteDataRomCommand(const u8* datab)
ez5h_sendWriteDataRomCommand:
	ldrh r1, [r0]
	adds r0, #2
	push {r0,r3}
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
	pop {r0,r3}
	mov pc, lr

.balign 4
send_writedata_data:
	.word EZ5H_CTRL_READ_0
	.word 0xF6B8
	.word REG_MCCNT0


.section "ez5h_write_sector", "ax"

.global ez5h_sdhc_write_label

@ bool ez5h_writeSector(u32 sector, void* buffer)
BEGIN_ASM_FUNC ez5h_writeSector
	push {r0-r1,r3,r4-r7,lr}

ez5h_sdhc_write_label:
	lsls r1, r0, #9

	movs r0, #0x58
	CALL_NO_INTERWORK SEND_SDIO_COMMAND_REG
	@ zero flag is set accordingly
	beq sdio_fail_write

	@ ez5h_sendSDIOCommand returned us EZ5H_CMD_SDMC_SEND_CLK(1) in r0-r1
	@ save low word of command
	movs r6, r0
	CALL_NO_INTERWORK SEND_COMMAND_REG

	@ we use lower short as value to write, upper short is EZ5H_CMD_SDMC_SEND_CRC_STATUS used below
	adr r0, write_tokens_label
	ldrh r5, [r0,#2]

	CALL_NO_INTERWORK SEND_WRITE_DATA_ROM_REG

	@ load buffer addr that was pushed at the start
	@ sdio4BitCrc16 will get the arguments directly from the stack
	@ and return the buffer address in r0
	CALL_NO_INTERWORK SDIO_CRC_REG

	@ write the data
	@ r0 is the data buffer provided by the above function call after dereferencing the input r0
	@ and it gets automatically incremented in ez5h_sendWriteDataRomCommand
	movs r3, #0xFF
1:
	CALL_NO_INTERWORK SEND_WRITE_DATA_ROM_REG
	@ do 0x100 iterations
	subs r3, #1
	bge 1b

	@ write the crc
	@ r0 gets automatically incremented in ez5h_sendWriteDataRomCommand
	mov r0, sp
	movs r3, #3
1:
	CALL_NO_INTERWORK SEND_WRITE_DATA_ROM_REG
	subs r3, #1
	bge 1b

	@ wait crc status start acknowledgment
	@ load EZ5H_CMD_SDMC_SEND_CRC_STATUS
	movs r1, #0
	movs r0, r5
1:
	CALL_NO_INTERWORK SEND_COMMAND_REG
	lsrs r2, #1
	bcs 1b

	@ send single crc read clock
	@ ===============================MAYBE BREAK====================
	@ CALL_NO_INTERWORK SEND_COMMAND_REG

	@ wait crc status acknowledged
1:
	CALL_NO_INTERWORK SEND_COMMAND_REG
	lsrs r2, #1
	bcc 1b

	@ wait for card to be ready again
	@ load backed up EZ5H_CMD_SDMC_SEND_CLK(1), r1 is already setup as 0 from before
	movs r0, r6
	movs r4, #0xFF
1:
	CALL_NO_INTERWORK SEND_COMMAND_REG
	tst r2, r4
	bne 1b

sdio_fail_write:
	@ r0 either is 0 or is EZ5H_CMD_SDMC_SEND_CLK(1) (thus nonzero)
	pop {r1-r2,r3,r4-r7,pc}
.balign 4
.pool
write_tokens_label:
	.word 0xF8B8F0FF
