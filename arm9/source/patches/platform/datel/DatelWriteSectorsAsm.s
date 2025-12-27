#include "asminc.h"

.syntax unified
.thumb

.section "datel_write", "ax"

.global DATEL_writeSectorSdhcLabel

@ All the called functions leave every registers unchanged, except for r0 in case the function has a return value

@DATEL_SDWriteMultipleSector(u32 sector, const u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC DATEL_SDWriteMultipleSector
    @ Among those regs, there's r2 being pushed, accessed afterwards from the stack
    push {r1-r7, lr}
    movs r4, r1
    @ We get the number of bytes total to write, which we'll compare to against in the main loop
    lsls r3, r2, #9

    @ Total written bytes
    movs r5, #0

DATEL_writeSectorSdhcLabel:
    @ if not sdhc this needs to be shifted to the left by 9
    lsls r0, #9
    @mov r0, r0

    @ this message needs 1 byte of extra clock before it starts waiting for the start token
    movs r1, DATEL_SDIO_CMD25_WRITE_MULTIPLE_BLOCK
    movs r2, #1
    ldr r7, DATEL_SDWriteMultipleSector_SpiSendSDIOCommand
    bl DATEL_SDWriteMultipleSector_Interwork
    bne CMD25_not_ok

    @ r6 contains DATEL_SDWriteMultipleSector_WaitSpiByteTimeout
    @ r7 contains DATEL_SDWriteMultipleSector_ReadSpiByte
    adr r1, DATEL_SDWriteMultipleSector_WaitSpiByteTimeout
    ldm r1!, {r6,r7}
    @ r1 contains DATEL_ReadWriteSpiByte
    adds r1, r7, #2

    @ We use the r2 set above to put 0x10000 to use later as write timeout
    @ it's 1 bigger than the timeout len but we save an instruction
    @ ldr     r2, =DATEL_SD_WRITE_TIMEOUT_LEN
    lsls r2, r2, #16

write_next_sector:

    @ Send start token
    movs r0, DATEL_SPI_MULTI_BLOCK_WRITE_TOKEN

    bl DATEL_SDWriteMultipleSector_InterworkR1 @ call DATEL_ReadWriteSpiByte

write_next_byte:
    ldrb r0, [r4, r5]
    bl DATEL_SDWriteMultipleSector_InterworkR1 @ call DATEL_ReadWriteSpiByte

    adds r5, #1
    @ Shifting left by 0x17 will set the Zero flag if the number that was shifted is a multiple
    @ of 0x200 (indicating a full sector has been written)
    lsls r0, r5, #0x17
    bne write_next_byte

    @ write dummy crc
    bl DATEL_SDWriteMultipleSector_Interwork @ call DATEL_ReadSpiByte
    bl DATEL_SDWriteMultipleSector_Interwork @ call DATEL_ReadSpiByte

    bl DATEL_SDWriteMultipleSector_Interwork @ call DATEL_ReadSpiByte

    @ we check if the lower nibble is equal to DATEL_SD_WRITE_OK
    subs r0, DATEL_SD_WRITE_OK
    lsls r0,#28
    bne write_command_failed

    @ Wait for card to write data
    bl DATEL_SDWriteMultipleSector_InterworkR6 @ call DATEL_WaitSpiByteTimeout
    beq sector_write_timeout_expired

    @ r3 holds the total number of bytes to write
    cmp r3, r5
    bne write_next_sector

    @ send stop token
    movs r0, DATEL_SPI_END_MULTI_BLOCK_WRITE
    bl DATEL_SDWriteMultipleSector_InterworkR1 @ call DATEL_ReadWriteSpiByte

    @ send 1 byte clock
    bl DATEL_SDWriteMultipleSector_Interwork @ call DATEL_ReadSpiByte

    bl DATEL_SDWriteMultipleSector_InterworkR6 @ call DATEL_WaitSpiByteTimeout

    @ pop {r1-r7, pc}

CMD25_not_ok:
write_command_failed:
sector_write_timeout_expired:
    @ movs r0, #0
    pop {r1-r7, pc}

DATEL_SDWriteMultipleSector_Interwork:
    bx r7
DATEL_SDWriteMultipleSector_InterworkR6:
    bx r6
DATEL_SDWriteMultipleSector_InterworkR1:
    bx r1
.balign 4
.pool

.global DATEL_SDWriteMultipleSector_SpiSendSDIOCommand
DATEL_SDWriteMultipleSector_SpiSendSDIOCommand:
    .word 0
.global DATEL_SDWriteMultipleSector_WaitSpiByteTimeout
DATEL_SDWriteMultipleSector_WaitSpiByteTimeout:
    .word 0
.global DATEL_SDWriteMultipleSector_ReadSpiByte
DATEL_SDWriteMultipleSector_ReadSpiByte:
    .word 0
