#include "asminc.h"

.syntax unified
.thumb

.section "datel_read", "ax"

.global datel_readSectorSdhcLabel

@ All the called functions leave every registers unchanged, except for r0 in case the function has a return value

@datel_SDReadMultipleSector(u32 sector, u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC datel_SDReadMultipleSector
    push {r3-r7, lr}
    movs r4, r1
    @ We get the number of bytes total to write, which we'll compare to against in the main loop
    lsls r3, r2, #9

    @ Total written bytes
    movs r5, #0

datel_readSectorSdhcLabel:
    @ if not sdhc this needs to be shifted to the left by 9
    lsls r0, #9
    @mov r0, r0

    movs r1, DATEL_SDIO_CMD18_READ_MULTIPLE_BLOCK

    ldr r7, datel_SDReadMultipleSector_SpiSendSDIOCommandR0
    bl datel_SDReadMultipleSector_Interwork
    bne CMD18_not_ok

read_next_sector:
    ldr r7, datel_SDReadMultipleSector_ReadSpiByteTimeout
    bl datel_SDReadMultipleSector_Interwork

    cmp r0, DATEL_SPI_START_DATA_TOKEN
    bne wrong_spi_start_token

    @ preload datel_readSpiShort
    ldr r7, datel_SDReadMultipleSector_ReadSpiShort
read_next_byte:
    bl datel_SDReadMultipleSector_Interwork
    strh r0, [r4, r5]

    adds r5, #2
    @ Shifting left by 0x17 will set the Zero flag if the number that was shifted is a multiple
    @ of 0x200 (indicating a full sector has been written)
    lsls r0, r5, #0x17
    bne read_next_byte

    @ drop crc
    bl datel_SDReadMultipleSector_Interwork

    cmp r3, r5
    bne read_next_sector

    movs r0, #0
    movs r1, DATEL_SDIO_CMD12_STOP_TRANSMISSION
    movs r2, #7

    @ datel_spiSendSDIOCommand is 1 instruction after datel_spiSendSDIOCommandR0
    ldr r7, datel_SDReadMultipleSector_SpiSendSDIOCommandR0
    adds r7, #2
    bl datel_SDReadMultipleSector_Interwork

    @ use a timeout of 0x1000 instead of 0xFFF, easier to setup, then halve it
	@ since we're reading 2 bytes at the time
    @ ldr r4, =DATEL_SD_CMD_TIMEOUT_LEN
    movs r4, #1
    lsls r4, #11
    ldr r7, datel_SDReadMultipleSector_ReadSpiShort
1:
    bl datel_SDReadMultipleSector_Interwork
    bne read_timeout_expired
    subs r4, #1
    bne 1b

read_timeout_expired:
    @ movs    r0, r4
    @ subs    r3, r0, #1
    @ sbcs    r0, r3

@ pico loader has no error result
    @ pop     {r3-r7, pc}

CMD18_not_ok:
wrong_spi_start_token:
    @ movs    r0, #0
    pop     {r3-r7, pc}

datel_SDReadMultipleSector_Interwork:
    bx r7
.balign 4
.pool

.global datel_SDReadMultipleSector_SpiSendSDIOCommandR0
datel_SDReadMultipleSector_SpiSendSDIOCommandR0:
    .word 0
.global datel_SDReadMultipleSector_ReadSpiByteTimeout
datel_SDReadMultipleSector_ReadSpiByteTimeout:
    .word 0
.global datel_SDReadMultipleSector_ReadSpiShort
datel_SDReadMultipleSector_ReadSpiShort:
    .word 0
