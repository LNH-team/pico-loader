#include "asminc.h"

.syntax unified
.thumb

.section "ards_write", "ax"

.global ARDS_writeSectorSdhcLabel

@ All the called functions leave every registers unchanged, except for r0 in case the function has a return value

@ARDS_SDWriteMultipleSector(u32 sector, const u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC ARDS_SDWriteMultipleSector
	@ Among those regs, there's r2 being pushed, accessed afterwards from the stack
	push    {r1-r7, lr}
	movs    r4, r1
	@ We get the number of bytes total to write, which we'll compare to against in the main loop
	lsls    r3, r2, #9

	@ Total written bytes
	movs    r5, #0

ARDS_writeSectorSdhcLabel:
	@ if not sdhc this needs to be shifted to the left by 9
	lsls    r0, #9
	@mov    r0, r0

	@ this message needs 1 byte of extra clock before it starts waiting for the start token
	movs    r1, ARDS_SDIO_CMD25_WRITE_MULTIPLE_BLOCK
	movs    r2, #1
	ldr     r7, ARDS_SDWriteMultipleSector_SpiSendSDIOCommand
	bl      ARDS_SDWriteMultipleSector_Interwork
	bne     CMD25_not_ok
	
	@ r6 contains ARDS_SDWriteMultipleSector_WaitSpiByteTimeout
	@ r7 contains ARDS_SDWriteMultipleSector_ReadSpiByte
	adr r1, ARDS_SDWriteMultipleSector_WaitSpiByteTimeout
	ldm r1!, {r6,r7}
	@ r1 contains ARDS_ReadWriteSpiByte
	adds    r1, r7, #2

	@ We use the r2 set above to put 0x10000 to use later as write timeout
	@ it's 1 bigger than the timeout len but we save an instruction
	@ ldr     r2, =ARDS_SD_WRITE_TIMEOUT_LEN	
	lsls    r2, r2, #16

write_next_sector:

	@ Send start token
	movs    r0, ARDS_SPI_MULTI_BLOCK_WRITE_TOKEN

	bl      ARDS_SDWriteMultipleSector_InterworkR1 @ call ARDS_ReadWriteSpiByte

write_next_byte:
	ldrb    r0, [r4, r5]
	bl      ARDS_SDWriteMultipleSector_InterworkR1 @ call ARDS_ReadWriteSpiByte

	adds    r5, #1
	@ Shifting left by 0x17 will set the Zero flag if the number that was shifted is a multiple
	@ of 0x200 (indicating a full sector has been written)
	lsls    r0, r5, #0x17
	bne     write_next_byte

	@ write dummy crc
	bl      ARDS_SDWriteMultipleSector_Interwork @ call ARDS_ReadSpiByte
	bl      ARDS_SDWriteMultipleSector_Interwork @ call ARDS_ReadSpiByte

	bl      ARDS_SDWriteMultipleSector_Interwork @ call ARDS_ReadSpiByte
	
	@ we check if the lower nibble is equal to ARDS_SD_WRITE_OK
	subs    r0, ARDS_SD_WRITE_OK
	lsls    r0,#28
	bne     write_command_failed

	@ Wait for card to write data
	bl      ARDS_SDWriteMultipleSector_InterworkR6 @ call ARDS_WaitSpiByteTimeout
	beq     sector_write_timeout_expired

	@ r3 holds the total number of bytes to write
	cmp     r3, r5
	bne     write_next_sector

	@ send stop token
	movs    r0, ARDS_SPI_END_MULTI_BLOCK_WRITE
	bl      ARDS_SDWriteMultipleSector_InterworkR1 @ call ARDS_ReadWriteSpiByte

	@ send 1 byte clock
	bl      ARDS_SDWriteMultipleSector_Interwork @ call ARDS_ReadSpiByte

	bl      ARDS_SDWriteMultipleSector_InterworkR6 @ call ARDS_WaitSpiByteTimeout

	@ pop     {r1-r7, pc}

CMD25_not_ok:
write_command_failed:
sector_write_timeout_expired:
	@ movs    r0, #0
	pop     {r1-r7, pc}

ARDS_SDWriteMultipleSector_Interwork:
	bx r7
ARDS_SDWriteMultipleSector_InterworkR6:
	bx r6
ARDS_SDWriteMultipleSector_InterworkR1:
	bx r1
.balign 4
.pool

.global ARDS_SDWriteMultipleSector_SpiSendSDIOCommand
ARDS_SDWriteMultipleSector_SpiSendSDIOCommand:
	.word 0
.global ARDS_SDWriteMultipleSector_WaitSpiByteTimeout
ARDS_SDWriteMultipleSector_WaitSpiByteTimeout:
	.word 0
.global ARDS_SDWriteMultipleSector_ReadSpiByte
ARDS_SDWriteMultipleSector_ReadSpiByte:
	.word 0
