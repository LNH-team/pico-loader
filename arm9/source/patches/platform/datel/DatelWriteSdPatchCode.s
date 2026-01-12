#include "asminc.h"

.syntax unified
.thumb

.section "datel_write", "ax"

.global datel_writeSectorSdhcLabel

@ All the called functions leave every registers unchanged, except for r0 in case the function has a return value

@datel_SDWriteMultipleSector(u32 sector, const u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC datel_SDWriteMultipleSector
    @ Among those regs, there's r2 being pushed, accessed afterwards from the stack
    push {r1-r7, lr}
    movs r4, r1
    @ We get the number of bytes total to write, which we'll compare to against in the main loop
    lsls r3, r2, #9

    @ Total written bytes
    movs r5, #0

datel_writeSectorSdhcLabel:
    @ if not sdhc this needs to be shifted to the left by 9
    lsls r0, #9
    @mov r0, r0

    @ this message needs 1 byte of extra clock before it starts waiting for the start token
    movs r1, DATEL_SDIO_CMD25_WRITE_MULTIPLE_BLOCK
    movs r2, #1
    ldr r7, datel_SDWriteMultipleSector_SpiSendSDIOCommand
    
    @ call datel_readSpiByte
    mov lr,pc
    mov pc,r7
    bne CMD25_not_ok

    @ r6 contains datel_SDWriteMultipleSector_WaitSpiByteTimeout
    @ r7 contains datel_SDWriteMultipleSector_ReadSpiByte
    adr r1, datel_SDWriteMultipleSector_WaitSpiByteTimeout
    ldm r1!, {r6,r7}
    @ r1 contains datel_readWriteSpiByte
    adds r1, r7, #2

    @ We use the r2 set above to put 0x10000 to use later as write timeout
    @ it's 1 bigger than the timeout len but we save an instruction
    @ ldr r2, =DATEL_SD_WRITE_TIMEOUT_LEN
    lsls r2, r2, #16

write_next_sector:

    @ Send start token
    movs r0, DATEL_SPI_MULTI_BLOCK_WRITE_TOKEN

    @ call datel_readWriteSpiByte
    mov lr,pc
    mov pc,r1

write_next_byte:
    ldrb r0, [r4, r5]
    @ call datel_readWriteSpiByte
    mov lr,pc
    mov pc,r1

    adds r5, #1
    @ Shifting left by 0x17 will set the Zero flag if the number that was shifted is a multiple
    @ of 0x200 (indicating a full sector has been written)
    lsls r0, r5, #0x17
    bne write_next_byte

    @ write dummy crc (ignore result)
    @ call datel_readSpiByte
    mov lr,pc
    mov pc,r7
    @ call datel_readSpiByte
    mov lr,pc
    mov pc,r7

    @ call datel_readSpiByte
    mov lr,pc
    mov pc,r7

    @ we check if the lower nibble is equal to DATEL_SD_WRITE_OK
    subs r0, DATEL_SD_WRITE_OK
    lsls r0,#28
    bne write_command_failed

    @ Wait for card to write data
    @ call datel_waitSpiByteTimeout
    mov lr,pc
    mov pc,r6
    beq sector_write_timeout_expired

    @ r3 holds the total number of bytes to write
    cmp r3, r5
    bne write_next_sector

    @ send stop token
    movs r0, DATEL_SPI_END_MULTI_BLOCK_WRITE
    
    @ call datel_readWriteSpiByte
    mov lr,pc
    mov pc,r1

    @ send 1 byte clock
    
    @ call datel_readSpiByte
    mov lr,pc
    mov pc,r7

    @ call datel_waitSpiByteTimeout
    mov lr,pc
    mov pc,r6
    
    @ pop {r1-r7, pc}

CMD25_not_ok:
write_command_failed:
sector_write_timeout_expired:
    @ movs r0, #0
    pop {r1-r7, pc}

.balign 4
.pool

.global datel_SDWriteMultipleSector_SpiSendSDIOCommand
datel_SDWriteMultipleSector_SpiSendSDIOCommand:
    .word 0
.global datel_SDWriteMultipleSector_WaitSpiByteTimeout
datel_SDWriteMultipleSector_WaitSpiByteTimeout:
    .word 0
.global datel_SDWriteMultipleSector_ReadSpiByte
datel_SDWriteMultipleSector_ReadSpiByte:
    .word 0
